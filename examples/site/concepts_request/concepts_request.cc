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

// [START functions_concepts_requests]
#include <google/cloud/functions/function.h>
#include <boost/beast.hpp>

namespace gcf = ::google::cloud::functions;

namespace {
// Use Boost.Beast to illustrate making HTTP requests, return the status code.
unsigned int make_http_request(std::string const& host);
}  // namespace

gcf::Function concepts_request() {
  return gcf::MakeFunction([](gcf::HttpRequest const& /*request*/) {
    std::string const host = "example.com";
    auto const code = make_http_request(host);
    return gcf::HttpResponse{}.set_payload(
        "Received code " + std::to_string(code) + " from " + host);
  });
}
// [END functions_concepts_requests]

namespace {
unsigned int make_http_request(std::string const& host) {
  namespace beast = boost::beast;
  namespace http = beast::http;
  using http_response = http::response<http::string_body>;
  using tcp = boost::asio::ip::tcp;

  // Create a socket to make the HTTP request over
  boost::asio::io_context ioc;
  tcp::resolver resolver(ioc);
  beast::tcp_stream stream(ioc);
  auto const results = resolver.resolve(host, "80");
  stream.connect(results);

  // Use Boost.Beast to make the HTTP request
  auto constexpr kHttpVersion = 10;  // 1.0 as Boost.Beast spells it
  http::request<http::string_body> req{http::verb::get, "/", kHttpVersion};
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  req.set(http::field::host, host);
  http::write(stream, req);
  beast::flat_buffer buffer;
  http_response res;
  http::read(stream, buffer, res);
  boost::system::error_code ec;
  stream.socket().shutdown(tcp::socket::shutdown_both, ec);
  (void)ec;  // ignore errors during shutdown()

  return res.result_int();
}
}  // namespace
