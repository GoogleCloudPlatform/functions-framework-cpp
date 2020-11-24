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

#include "google/cloud/functions/internal/framework.h"
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast.hpp>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <gmock/gmock.h>
#include <chrono>
#include <string>

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {
namespace {

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
  auto constexpr kInitialDelay = std::chrono::milliseconds(100);
  auto constexpr kAttempts = 5;
  auto delay = kInitialDelay;
  for (int i = 0; i != kAttempts; ++i) {
    std::this_thread::sleep_for(delay);
    try {
      (void)HttpGet(host, port, "/ok");
      return 0;
    } catch (std::exception const& ex) {
      std::cerr << "WaitForServerReady[" << i << "]: failed with " << ex.what()
                << std::endl;
    }
    delay *= 2;
  }
  return -1;
}

char const* argv0 = nullptr;

TEST(RunIntegrationTest, Basic) {
  auto constexpr kOkay = 200;
  auto constexpr kNotFound = 404;
  auto constexpr kInternalError = 500;

  auto const exe = bfs::path(argv0).parent_path() / "echo_server";
  auto server = bp::child(exe, "--port=8080");
  auto result = WaitForServerReady("localhost", "8080");
  ASSERT_EQ(result, 0);

  auto actual = HttpGet("localhost", "8080", "/say/hello");
  EXPECT_EQ(actual.result_int(), kOkay);
  EXPECT_THAT(actual.body(), HasSubstr(R"js("target": "/say/hello")js"));

  actual = HttpGet("localhost", "8080", "/error/" + std::to_string(kNotFound));
  EXPECT_THAT(actual.result_int(), kNotFound);

  actual = HttpGet("localhost", "8080", "/exception/");
  EXPECT_THAT(actual.result_int(), kInternalError);

  try {
    (void)HttpGet("localhost", "8080", "/quit/program/0");
  } catch (...) {
  }
  server.wait();
  EXPECT_EQ(server.exit_code(), 0);
}

TEST(RunIntegrationTest, ExceptionLogsToStderr) {
  auto constexpr kInternalError = 500;

  auto const exe = bfs::path(argv0).parent_path() / "echo_server";
  bp::ipstream child_stderr;
  auto server = bp::child(exe, "--port=8080", bp::std_err > child_stderr);
  auto result = WaitForServerReady("localhost", "8080");
  ASSERT_EQ(result, 0);

  auto actual = HttpGet("localhost", "8080", "/exception/test-string");
  EXPECT_THAT(actual.result_int(), kInternalError);

  std::string line;
  std::getline(child_stderr, line);
  EXPECT_THAT(line, HasSubstr("standard C++ exception"));
  EXPECT_THAT(line, HasSubstr("/exception/test-string"));

  try {
    (void)HttpGet("localhost", "8080", "/quit/program/0");
  } catch (...) {
  }
  server.wait();
  EXPECT_EQ(server.exit_code(), 0);
}

TEST(RunIntegrationTest, OutputIsFlushed) {
  auto constexpr kOkay = 200;

  auto const exe = bfs::path(argv0).parent_path() / "echo_server";
  bp::ipstream child_stderr;
  bp::ipstream child_stdout;
  auto server = bp::child(exe, "--port=8080", bp::std_err > child_stderr,
                          bp::std_out > child_stdout);
  auto result = WaitForServerReady("localhost", "8080");
  ASSERT_EQ(result, 0);

  std::string line;

  auto actual = HttpGet("localhost", "8080", "/buffered-stdout/test-string");
  EXPECT_THAT(actual.result_int(), kOkay);
  EXPECT_TRUE(std::getline(child_stdout, line));
  EXPECT_THAT(line, HasSubstr("stdout:"));
  EXPECT_THAT(line, HasSubstr("/buffered-stdout/test-string"));

  actual = HttpGet("localhost", "8080", "/buffered-stderr/test-string");
  EXPECT_THAT(actual.result_int(), kOkay);
  EXPECT_TRUE(std::getline(child_stderr, line));
  EXPECT_THAT(line, HasSubstr("stderr:"));
  EXPECT_THAT(line, HasSubstr("/buffered-stderr/test-string"));

  try {
    (void)HttpGet("localhost", "8080", "/quit/program/0");
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
