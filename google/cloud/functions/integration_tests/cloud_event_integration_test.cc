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
#include "google/cloud/functions/http_response.h"
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast.hpp>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <gmock/gmock.h>
#include <nlohmann/json.hpp>
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

http_response HttpPost(std::string const& host, std::string const& port,
                       std::string const& subject,
                       std::string const& target = "/") {
  using tcp = boost::asio::ip::tcp;

  // Create a socket to make the HTTP request over
  boost::asio::io_context ioc;
  tcp::resolver resolver(ioc);
  beast::tcp_stream stream(ioc);
  auto const results = resolver.resolve(host, port);
  stream.connect(results);

  // Use Boost.Beast to make the HTTP request
  auto constexpr kHttpVersion = 10;  // 1.0 as Boost.Beast spells it
  http::request<http::string_body> req{http::verb::post, target, kHttpVersion};
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  req.set(http::field::host, host);
  req.set(http::field::content_type, "application/cloudevents+json");
  auto const id = boost::uuids::random_generator()();
  auto json = nlohmann::json{
      {"type", "com.github/GoogleCloudPlatform/test-message"},
      {"source", "/test-program"},
      {"id", boost::uuids::to_string(id)},
      {"subject", subject},
  };
  req.body() = json.dump();
  req.prepare_payload();
  http::write(stream, req);
  beast::flat_buffer buffer;
  http_response res;
  http::read(stream, buffer, res);
  stream.socket().shutdown(tcp::socket::shutdown_both);

  return res;
}

http_response HttpPutBatch(std::string const& host, std::string const& port,
                           std::string const& subject) {
  using tcp = boost::asio::ip::tcp;

  // Create a socket to make the HTTP request over
  boost::asio::io_context ioc;
  tcp::resolver resolver(ioc);
  beast::tcp_stream stream(ioc);
  auto const results = resolver.resolve(host, port);
  stream.connect(results);

  // Use Boost.Beast to make the HTTP request
  auto constexpr kHttpVersion = 10;  // 1.0 as Boost.Beast spells it
  http::request<http::string_body> req{http::verb::put, "/", kHttpVersion};
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  req.set(http::field::host, host);
  req.set(http::field::content_type, "application/cloudevents-batch+json");
  auto json = nlohmann::json::array();
  std::generate_n(std::back_inserter(json), 3, [subject] {
    auto const id = boost::uuids::random_generator()();
    return nlohmann::json{
        {"type", "com.github/GoogleCloudPlatform/test-message"},
        {"source", "/test-program"},
        {"id", boost::uuids::to_string(id)},
        {"subject", subject},
    };
  });
  req.body() = json.dump();
  req.prepare_payload();
  http::write(stream, req);
  beast::flat_buffer buffer;
  http_response res;
  http::read(stream, buffer, res);
  stream.socket().shutdown(tcp::socket::shutdown_both);

  return res;
}

http_response HttpGetBinary(std::string const& host, std::string const& port) {
  using tcp = boost::asio::ip::tcp;

  // Create a socket to make the HTTP request over
  boost::asio::io_context ioc;
  tcp::resolver resolver(ioc);
  beast::tcp_stream stream(ioc);
  auto const results = resolver.resolve(host, port);
  stream.connect(results);

  // Use Boost.Beast to make the HTTP request
  auto constexpr kHttpVersion = 10;  // 1.0 as Boost.Beast spells it
  http::request<http::string_body> req{http::verb::get, "/", kHttpVersion};
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  req.set(http::field::host, host);
  req.set(http::field::content_type, "text/plain");
  req.set("ce-type", "com.github/GoogleCloudPlatform/test-message");
  req.set("ce-source", "/test-program");
  auto const id = boost::uuids::random_generator()();
  req.set("ce-id", boost::uuids::to_string(id));
  req.body() = "some data\n";
  req.prepare_payload();
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
      (void)HttpPost(host, port, "ping");
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

auto constexpr kServer = "cloud_event_handler";

TEST(RunIntegrationTest, Basic) {
  auto const exe = bfs::path(argv0).parent_path() / kServer;
  auto server = bp::child(exe, "--port=8080");
  auto result = WaitForServerReady("localhost", "8080");
  ASSERT_EQ(result, 0);

  auto actual = HttpPost("localhost", "8080", "hello");
  EXPECT_EQ(actual.result_int(), functions::HttpResponse::kOkay);

  actual = HttpPost("localhost", "8080", "/exception/");
  EXPECT_THAT(actual.result_int(),
              functions::HttpResponse::kInternalServerError);

  actual = HttpPost("localhost", "8080", "/unknown-exception/");
  EXPECT_THAT(actual.result_int(),
              functions::HttpResponse::kInternalServerError);

  actual = HttpPost("localhost", "8080", "none", "/favicon.ico");
  EXPECT_THAT(actual.result_int(), functions::HttpResponse::kNotFound);

  actual = HttpPost("localhost", "8080", "none", "/robots.txt");
  EXPECT_THAT(actual.result_int(), functions::HttpResponse::kNotFound);

  try {
    (void)HttpPost("localhost", "8080", "/quit/program/0");
  } catch (...) {
  }
  server.wait();
  EXPECT_EQ(server.exit_code(), 0);
}

TEST(RunIntegrationTest, Batch) {
  auto const exe = bfs::path(argv0).parent_path() / kServer;
  auto server = bp::child(exe, "--port=8080");
  auto result = WaitForServerReady("localhost", "8080");
  ASSERT_EQ(result, 0);

  auto actual = HttpPutBatch("localhost", "8080", "hello");
  EXPECT_EQ(actual.result_int(), functions::HttpResponse::kOkay);
  try {
    (void)HttpPost("localhost", "8080", "/quit/program/0");
  } catch (...) {
  }
  server.wait();
  EXPECT_EQ(server.exit_code(), 0);
}

TEST(RunIntegrationTest, Binary) {
  auto const exe = bfs::path(argv0).parent_path() / kServer;
  auto server = bp::child(exe, "--port=8080");
  auto result = WaitForServerReady("localhost", "8080");
  ASSERT_EQ(result, 0);

  auto actual = HttpGetBinary("localhost", "8080");
  EXPECT_EQ(actual.result_int(), functions::HttpResponse::kOkay);
  try {
    (void)HttpPost("localhost", "8080", "/quit/program/0");
  } catch (...) {
  }
  server.wait();
  EXPECT_EQ(server.exit_code(), 0);
}

TEST(RunIntegrationTest, ExceptionLogsToStderr) {
  auto const exe = bfs::path(argv0).parent_path() / kServer;
  bp::ipstream child_stderr;
  auto server = bp::child(exe, "--port=8080", bp::std_err > child_stderr);
  auto result = WaitForServerReady("localhost", "8080");
  ASSERT_EQ(result, 0);

  auto actual = HttpPost("localhost", "8080", "/exception/test-string");
  EXPECT_THAT(actual.result_int(),
              functions::HttpResponse::kInternalServerError);

  std::string line;
  std::getline(child_stderr, line);
  EXPECT_THAT(line, HasSubstr("standard C++ exception"));
  EXPECT_THAT(line, HasSubstr("/exception/test-string"));

  try {
    (void)HttpPost("localhost", "8080", "/quit/program/0");
  } catch (...) {
  }
  server.wait();
  EXPECT_EQ(server.exit_code(), 0);
}

TEST(RunIntegrationTest, OutputIsFlushed) {
  auto const exe = bfs::path(argv0).parent_path() / kServer;
  bp::ipstream child_stderr;
  bp::ipstream child_stdout;
  auto server = bp::child(exe, "--port=8080", bp::std_err > child_stderr,
                          bp::std_out > child_stdout);
  auto result = WaitForServerReady("localhost", "8080");
  ASSERT_EQ(result, 0);

  std::string line;

  auto actual = HttpPost("localhost", "8080", "/buffered-stdout/test-string");
  EXPECT_THAT(actual.result_int(), functions::HttpResponse::kOkay);
  EXPECT_TRUE(std::getline(child_stdout, line));
  EXPECT_THAT(line, HasSubstr("stdout:"));
  EXPECT_THAT(line, HasSubstr("/buffered-stdout/test-string"));

  actual = HttpPost("localhost", "8080", "/buffered-stderr/test-string");
  EXPECT_THAT(actual.result_int(), functions::HttpResponse::kOkay);
  EXPECT_TRUE(std::getline(child_stderr, line));
  EXPECT_THAT(line, HasSubstr("stderr:"));
  EXPECT_THAT(line, HasSubstr("/buffered-stderr/test-string"));

  try {
    (void)HttpPost("localhost", "8080", "/quit/program/0");
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
