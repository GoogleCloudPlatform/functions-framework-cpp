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

#include "google/cloud/functions/internal/framework_impl.h"
#include "google/cloud/functions/framework.h"
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast.hpp>
#include <gmock/gmock.h>
#include <future>
#include <string>

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {
namespace {

using ::testing::IsEmpty;

char const* const kTestArgv[] = {"unused", "--port=0"};
auto constexpr kTestArgc = sizeof(kTestArgv) / sizeof(kTestArgv[0]);

char const* const kTestInvalidArgv[] = {"unused", "--port=12345467"};
auto constexpr kTestInvalidArgc =
    sizeof(kTestInvalidArgv) / sizeof(kTestInvalidArgv[0]);

std::string HttpGet(std::string const& host, std::string const& port,
                    std::string const& target) {
  namespace beast = boost::beast;
  namespace http = beast::http;
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
  req.prepare_payload();
  http::write(stream, req);
  beast::flat_buffer buffer;
  http::response<http::string_body> res;
  http::read(stream, buffer, res);
  stream.socket().shutdown(tcp::socket::shutdown_both);

  // Return the body.
  return std::move(res.body());
}

std::string CloudEventGet(std::string const& host, std::string const& port,
                          std::string const& target) {
  namespace beast = boost::beast;
  namespace http = beast::http;
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
  auto constexpr kText = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234"})js";
  req.set(http::field::content_type, "application/cloudevents+json");
  req.body() = kText;
  req.prepare_payload();
  http::write(stream, req);
  beast::flat_buffer buffer;
  http::response<http::string_body> res;
  http::read(stream, buffer, res);
  stream.socket().shutdown(tcp::socket::shutdown_both);

  // Return the body.
  return std::move(res.body());
}

TEST(FrameworkTest, Http) {
  std::promise<int> port_p;
  auto port_f = port_p.get_future();
  std::atomic<bool> shutdown{false};
  auto hello = [](functions::HttpRequest const& r) {
    functions::HttpResponse response;
    response.set_payload("Hello World from " + r.target());
    response.set_header("content-type", "text/plain");
    return response;
  };
  auto run = [&](int argc, char const* const argv[],
                 functions::UserHttpFunction f) {
    return RunForTest(
        argc, argv, std::move(f), [&shutdown]() { return shutdown.load(); },
        [&port_p](int port) mutable { port_p.set_value(port); });
  };
  auto done = std::async(std::launch::async, run, kTestArgc, kTestArgv, hello);

  auto port = port_f.get();
  auto actual = HttpGet("localhost", std::to_string(port), "/say/hello");
  EXPECT_EQ(actual, "Hello World from /say/hello");
  shutdown.store(true);
  // Making a second request guarantees the change in `shutdown` is seen, but
  // can fail.
  try {
    (void)HttpGet("localhost", std::to_string(port), "/quit/now");
  } catch (...) {
  }
  EXPECT_EQ(done.get(), 0);
}

TEST(FrameworkTest, HttpInvalidPort) {
  auto const exit_code = ::google::cloud::functions::Run(
      kTestInvalidArgc, kTestInvalidArgv, functions::UserHttpFunction{});
  EXPECT_NE(exit_code, 0);
}

TEST(FrameworkTest, CloudEvent) {
  std::promise<int> port_p;
  auto port_f = port_p.get_future();
  std::atomic<bool> shutdown{false};
  auto hello = [](functions::CloudEvent const& /*event*/) {};
  auto run = [&](int argc, char const* const argv[],
                 functions::UserCloudEventFunction f) {
    return RunForTest(
        argc, argv, std::move(f), [&shutdown]() { return shutdown.load(); },
        [&port_p](int port) mutable { port_p.set_value(port); });
  };
  auto done = std::async(std::launch::async, run, kTestArgc, kTestArgv, hello);

  auto port = port_f.get();
  auto actual = CloudEventGet("localhost", std::to_string(port), "/");
  EXPECT_THAT(actual, IsEmpty());
  shutdown.store(true);
  // Making a second request guarantees the change in `shutdown` is seen, but
  // can fail.
  try {
    (void)HttpGet("localhost", std::to_string(port), "/quit/now");
  } catch (...) {
  }
  EXPECT_EQ(done.get(), 0);
}

TEST(FrameworkTest, CloudEventInvalidPort) {
  auto const exit_code = ::google::cloud::functions::Run(
      kTestInvalidArgc, kTestInvalidArgv, functions::UserCloudEventFunction{});
  EXPECT_NE(exit_code, 0);
}

}  // namespace
}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal
