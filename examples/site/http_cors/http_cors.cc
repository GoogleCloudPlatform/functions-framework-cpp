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

// [START functions_http_cors]
#include <google/cloud/functions/http_request.h>
#include <google/cloud/functions/http_response.h>

namespace gcf = ::google::cloud::functions;

gcf::HttpResponse http_cors(gcf::HttpRequest request) {  // NOLINT
  // Set CORS headers for preflight request
  if (request.verb() == "OPTIONS") {
    // Allows GET requests from any origin with the Content-Type header and
    // caches preflight response for an 3600s
    gcf::HttpResponse response;
    response.set_result(gcf::HttpResponse::kNoContent);
    response.set_header("Access-Control-Allow-Origin", "*");
    response.set_header("Access-Control-Allow-Methods", "GET");
    response.set_header("Access-Control-Allow-Headers", "Content-Type");
    response.set_header("Access-Control-Max-Age", "3600");
    return response;
  }

  gcf::HttpResponse response;
  response.set_header("content-type", "text/plain");
  response.set_payload("Hello World!");
  response.set_header("Access-Control-Allow-Origin", "*");
  return response;
}
// [END functions_http_cors]
