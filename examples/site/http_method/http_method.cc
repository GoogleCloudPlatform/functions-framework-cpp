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

// [START functions_http_method]
#include <google/cloud/functions/function.h>

namespace gcf = ::google::cloud::functions;

gcf::Function http_method() {
  return gcf::MakeFunction([](gcf::HttpRequest const& request) {
    if (request.verb() == "GET") {
      return gcf::HttpResponse{}
          .set_header("content-type", "text/plain")
          .set_payload("Hello World!");
    }

    return gcf::HttpResponse{}.set_result(
        request.verb() == "POST" ? gcf::HttpResponse::kForbidden
                                 : gcf::HttpResponse::kMethodNotAllowed);
  });
}
// [END functions_http_method]
