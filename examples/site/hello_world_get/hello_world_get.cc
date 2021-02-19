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

// [START functions_helloworld_get]
#include <google/cloud/functions/http_request.h>
#include <google/cloud/functions/http_response.h>

namespace gcf = ::google::cloud::functions;

gcf::HttpResponse hello_world_get(gcf::HttpRequest) {  // NOLINT
  gcf::HttpResponse response;
  response.set_payload("Hello World!");
  response.set_header("content-type", "text/plain");
  return response;
}
// [END functions_helloworld_get]
