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

// [START functions_http_integration_test]
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast.hpp>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <gmock/gmock.h>
#include <chrono>
#include <string>

namespace {

namespace bp = boost::process;
namespace bfs = boost::filesystem;  // Boost.Process cannot use std::filesystem
namespace beast = boost::beast;
namespace http = beast::http;
using http_response = http::response<http::string_body>;

// Wait until an HTTP server starts responding.
int WaitForServerReady(std::string const& host, std::string const& port);
http_response HttpGet(std::string const& host, std::string const& port,
                      std::string const& target, std::string payload);

char const* argv0 = nullptr;

class HttpIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    auto const exe = bfs::path(argv0).parent_path() / "http_integration_server";
    auto server = bp::child(exe, "--port=8080");
    auto result = WaitForServerReady("localhost", "8080");
    ASSERT_EQ(result, 0);
    process_ = std::move(server);
  }

  void TearDown() override {
    process_.terminate();
    process_.wait();
  }

 private:
  bp::child process_;
};

TEST_F(HttpIntegrationTest, Basic) {
  auto constexpr kOkay = 200;

  auto actual = HttpGet("localhost", "8080", "/", R"js({"name": "Foo"})js");
  EXPECT_EQ(actual.result_int(), kOkay);
  EXPECT_EQ(actual.body(), "Hello Foo!");

  actual = HttpGet("localhost", "8080", "/", R"js({})js");
  EXPECT_EQ(actual.result_int(), kOkay);
  EXPECT_EQ(actual.body(), "Hello World!");
}

http_response HttpGet(std::string const& host, std::string const& port,
                      std::string const& target, std::string payload) {
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
  req.body() = std::move(payload);
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
      (void)HttpGet(host, port, "/", "{}");
      return 0;
    } catch (std::exception const& ex) {
      std::cerr << "WaitForServerReady[" << i << "]: failed with " << ex.what()
                << std::endl;
    }
    delay *= 2;
  }
  return -1;
}

}  // namespace

int main(int argc, char* argv[]) {
  ::testing::InitGoogleMock(&argc, argv);
  ::argv0 = argv[0];
  return RUN_ALL_TESTS();
}

// [START functions_http_integration_test]
