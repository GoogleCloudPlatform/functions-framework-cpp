// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "google/cloud/functions/framework.h"
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast.hpp>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <gmock/gmock.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <chrono>
#include <future>
#include <string>
#include <thread>

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {
namespace {

using ::testing::Each;
using ::testing::HasSubstr;

namespace bp = boost::process;
// Even with C++17, we have to use the Boost version because Boost.Process
// expects it.
namespace bfs = boost::filesystem;
namespace beast = boost::beast;
namespace http = beast::http;
using http_response = http::response<http::string_body>;

http_response HttpGet(std::string const& host, std::string const& port,
                      std::string const& target) {
  using tcp = boost::asio::ip::tcp;

  // Create a socket to make the HTTP request over
  boost::asio::io_context ioc;
  tcp::resolver resolver(ioc);
  beast::tcp_stream stream(ioc);
  auto const results = resolver.resolve(host, port);
  stream.connect(results);

  // Use Boost.Beast to make the HTTP request
  auto constexpr kHttpVersion = 10;  // 1.0 as Boost.Beast spells it
  http::request<http::string_body> req{http::verb::get, target, kHttpVersion};
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  req.set(http::field::host, host);
  http::write(stream, req);
  beast::flat_buffer buffer;
  http_response res;
  http::read(stream, buffer, res);
  stream.socket().shutdown(tcp::socket::shutdown_both);

  return res;
}

int WaitForServerReady(std::string const& host, std::string const& port) {
  using namespace std::chrono_literals;
  for (auto delay : {100ms, 200ms, 400ms, 800ms, 1600ms, 3200ms}) {  // NOLINT
    std::cout << "Waiting for server to start [" << delay.count() << "ms]\n";
    std::this_thread::sleep_for(delay);
    try {
      auto const r = HttpGet(host, port, "/ok");
      if (r.result() == beast::http::status::ok) return 0;
      std::cerr << "... [" << r.result_int() << "]" << std::endl;
    } catch (std::exception const& ex) {
      std::cerr << "WaitForServerReady[" << delay.count() << "ms]: failed with "
                << ex.what() << std::endl;
    }
    delay *= 2;
  }
  return -1;
}

char const* argv0 = nullptr;

auto constexpr kServer = "echo_server";
auto constexpr kConformanceServer = "http_conformance";

auto ExePath(bfs::path const& filename) {
  static auto const kPath = std::vector<bfs::path>{
      bfs::canonical(argv0).make_preferred().parent_path()};
  return bp::search_path(filename, kPath);
}

TEST(RunIntegrationTest, Basic) {
  auto server = bp::child(ExePath(kServer), "--port=8010");
  auto result = WaitForServerReady("localhost", "8010");
  ASSERT_EQ(result, 0);

  auto actual = HttpGet("localhost", "8010", "/say/hello");
  EXPECT_EQ(actual.result_int(), functions::HttpResponse::kOkay);
  EXPECT_THAT(actual.body(), HasSubstr(R"js("target": "/say/hello")js"));

  actual =
      HttpGet("localhost", "8010",
              "/error/" + std::to_string(functions::HttpResponse::kNotFound));
  EXPECT_THAT(actual.result_int(), functions::HttpResponse::kNotFound);

  actual = HttpGet("localhost", "8010", "/exception/");
  EXPECT_THAT(actual.result_int(),
              functions::HttpResponse::kInternalServerError);

  actual = HttpGet("localhost", "8010", "/unknown-exception/");
  EXPECT_THAT(actual.result_int(),
              functions::HttpResponse::kInternalServerError);

  actual = HttpGet("localhost", "8010", "/favicon.ico");
  EXPECT_THAT(actual.result_int(), functions::HttpResponse::kNotFound);

  actual = HttpGet("localhost", "8010", "/robots.txt");
  EXPECT_THAT(actual.result_int(), functions::HttpResponse::kNotFound);

  try {
    (void)HttpGet("localhost", "8010", "/quit/program/0");
    (void)HttpGet("localhost", "8010", "/quit/program/0");
  } catch (...) {
  }
  server.wait();
  EXPECT_EQ(server.exit_code(), 0);
}

TEST(RunIntegrationTest, ExceptionLogsToStderr) {
  bp::ipstream child_stderr;
  auto server =
      bp::child(ExePath(kServer), "--port=8010", bp::std_err > child_stderr);
  auto result = WaitForServerReady("localhost", "8010");
  ASSERT_EQ(result, 0);

  auto actual = HttpGet("localhost", "8010", "/exception/test-string");
  EXPECT_THAT(actual.result_int(),
              functions::HttpResponse::kInternalServerError);

  std::string line;
  std::getline(child_stderr, line);
  auto log = nlohmann::json::parse(line, /*cb=*/nullptr,
                                   /*allow_exceptions=*/false);
  ASSERT_TRUE(log.is_object());
  EXPECT_EQ(log.value("severity", ""), "error");
  EXPECT_THAT(log.value("message", ""), HasSubstr("standard C++ exception"));
  EXPECT_THAT(log.value("message", ""), HasSubstr("/exception/test-string"));

  try {
    (void)HttpGet("localhost", "8010", "/quit/program/0");
    (void)HttpGet("localhost", "8010", "/quit/program/0");
  } catch (...) {
  }
  server.wait();
  EXPECT_EQ(server.exit_code(), 0);
}

TEST(RunIntegrationTest, OutputIsFlushed) {
  bp::ipstream child_stderr;
  bp::ipstream child_stdout;
  auto server =
      bp::child(ExePath(kServer), "--port=8010", bp::std_err > child_stderr,
                bp::std_out > child_stdout);
  auto result = WaitForServerReady("localhost", "8010");
  ASSERT_EQ(result, 0);

  std::string line;

  auto actual = HttpGet("localhost", "8010", "/buffered-stdout/test-string");
  EXPECT_THAT(actual.result_int(), functions::HttpResponse::kOkay);
  EXPECT_TRUE(std::getline(child_stdout, line));
  EXPECT_THAT(line, HasSubstr("stdout:"));
  EXPECT_THAT(line, HasSubstr("/buffered-stdout/test-string"));

  actual = HttpGet("localhost", "8010", "/buffered-stderr/test-string");
  EXPECT_THAT(actual.result_int(), functions::HttpResponse::kOkay);
  EXPECT_TRUE(std::getline(child_stderr, line));
  EXPECT_THAT(line, HasSubstr("stderr:"));
  EXPECT_THAT(line, HasSubstr("/buffered-stderr/test-string"));

  try {
    (void)HttpGet("localhost", "8010", "/quit/program/0");
    (void)HttpGet("localhost", "8010", "/quit/program/0");
  } catch (...) {
  }
  server.wait();
  EXPECT_EQ(server.exit_code(), 0);
}

TEST(RunIntegrationTest, ConformanceSmokeTest) {
  auto server = bp::child(ExePath(kConformanceServer), "--port=8010");
  auto result = WaitForServerReady("localhost", "8010");
  ASSERT_EQ(result, 0);

  try {
    (void)HttpGet("localhost", "8010", "/quit/program/0");
    (void)HttpGet("localhost", "8010", "/quit/program/0");
  } catch (...) {
  }
  server.wait();
  EXPECT_EQ(server.exit_code(), 0);
}

TEST(RunIntegrationTest, SomeParallelism) {
  auto server = bp::child(ExePath(kServer), "--port=8010");
  auto result = WaitForServerReady("localhost", "8010");
  ASSERT_EQ(result, 0);

  auto constexpr kTaskCount = 16;
  auto task = [] {
    auto start = std::chrono::steady_clock::now();
    (void)HttpGet("localhost", "8010", "/sleep/1000");
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
  };
  std::vector<std::future<std::chrono::milliseconds>> tasks(kTaskCount);
  std::generate(tasks.begin(), tasks.end(),
                [&] { return std::async(std::launch::async, task); });
  std::vector<std::chrono::milliseconds> elapsed(tasks.size());
  std::transform(tasks.begin(), tasks.end(), elapsed.begin(),
                 [](auto& f) { return f.get(); });
  // We ask for a 1s delay, but we tolerate up to 5.
  EXPECT_THAT(elapsed, Each(::testing::Le(std::chrono::seconds(5))));

  try {
    (void)HttpGet("localhost", "8010", "/quit/program/0");
    (void)HttpGet("localhost", "8010", "/quit/program/0");
  } catch (...) {
  }
  server.wait();
  EXPECT_EQ(server.exit_code(), 0);
}

}  // namespace
}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal

int main(int argc, char* argv[]) {
  ::testing::InitGoogleMock(&argc, argv);
  google::cloud::functions_internal::argv0 = argv[0];
  return RUN_ALL_TESTS();
}
