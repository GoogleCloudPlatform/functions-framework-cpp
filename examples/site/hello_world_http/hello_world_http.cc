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

// [START functions_helloworld_http]
#include <google/cloud/functions/function.h>
#include <nlohmann/json.hpp>

namespace gcf = ::google::cloud::functions;

gcf::HttpResponse hello_world_http_impl(gcf::HttpRequest request) {
  auto greeting = [r = std::move(request)] {
    auto request_json = nlohmann::json::parse(r.payload(), /*cb=*/nullptr,
                                              /*allow_exceptions=*/false);
    if (request_json.count("name") && request_json["name"].is_string()) {
      return "Hello " + request_json.value("name", "World") + "!";
    }
    return std::string("Hello World!");
  };

  return gcf::HttpResponse{}
      .set_header("content-type", "text/plain")
      .set_payload(greeting());
}

gcf::Function hello_world_http() {
  return gcf::MakeFunction(hello_world_http_impl);
}
// [END functions_helloworld_http]
