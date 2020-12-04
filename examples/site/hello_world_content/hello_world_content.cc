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

// [START functions_hello_world_content]
#include <google/cloud/functions/http_request.h>
#include <google/cloud/functions/http_response.h>
#include <nlohmann/json.hpp>
#include <map>
#include <string>

namespace gcf = ::google::cloud::functions;

std::map<std::string, std::string> parse_www_form_urlencoded(
    std::string const& text);

gcf::HttpResponse hello_world_content_type(
    gcf::HttpRequest request) {  // NOLINT
  std::string name;
  auto const& headers = request.headers();
  if (auto f = headers.find("content-type"); f != headers.end()) {
    if (f->second == "application/json") {
      name = nlohmann::json(request.payload()).value("name", "");
    } else if (f->second == "application/octet-stream") {
      name = request.payload();  // treat contents as a string
    } else if (f->second == "text/plain") {
      name = request.payload();
    } else if (f->second == "application/x-www-form-urlencoded'") {
      // Use your preferred parser, here we use some custom code.
      auto form = parse_www_form_urlencoded(request.payload());
      name = form["name"];
    }
  }

  gcf::HttpResponse response;
  response.set_payload("Hello " + name);
  response.set_header("content-type", "text/plain");
  return response;
}
// [END functions_hello_world_content]
