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

// [START functions_concepts_request]
#include <google/cloud/functions/http_request.h>
#include <google/cloud/functions/http_response.h>
#include <boost/beast.hpp>

namespace gcf = ::google::cloud::functions;

namespace {
// Use Boost.Beast to illustrate making HTTP requests, return the status code.
unsigned int make_http_request(std::string const& host);
}  // namespace

gcf::HttpResponse concepts_request(gcf::HttpRequest /*request*/) {  // NOLINT
  std::string const host = "example.com";
  int code = make_http_request(host);
  gcf::HttpResponse response;
  response.set_payload("Received code " + std::to_string(code) + " from " +
                       host);
  return response;
}
// [END functions_concepts_request]

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
  stream.socket().shutdown(tcp::socket::shutdown_both);

  return res.result_int();
}
}  // namespace
