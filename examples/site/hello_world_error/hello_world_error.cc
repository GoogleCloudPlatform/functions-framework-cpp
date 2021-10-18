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

// [START functions_helloworld_error]
#include <google/cloud/functions/http_request.h>
#include <google/cloud/functions/http_response.h>
#include <nlohmann/json.hpp>
#include <iostream>

namespace gcf = ::google::cloud::functions;

// Though not used in this example, the request is passed by value to support
// applications that move-out its data.
gcf::HttpResponse hello_world_error(gcf::HttpRequest request) {  // NOLINT
  if (request.target() == "/return500") {
    // An error response code does NOT create entries in Error Reporting
    return gcf::HttpResponse{}.set_result(
        gcf::HttpResponse::kInternalServerError);
  }
  // Unstructured logs to stdout and/or stderr do NOT create entries in Error
  // Reporting
  std::cout << "An error occurred (stdout)\n";
  std::cerr << "An error occurred (stderr)\n";

  if (request.target() == "/throw/exception") {
    // Throwing an exception WILL create new entries in Error Reporting
    throw std::runtime_error("I failed you");
  }

  // Structured logs MAY create entries in Error Reporting depending on their
  // severity. You can create structured logs manually (as shown here), or using
  // your favorite logging library with suitable formatting.
  std::cerr << nlohmann::json{{"severity", "info"},
                              {"message", "informational message"}}
                   .dump()
            << std::endl;

  std::cerr << nlohmann::json{{"severity", "error"},
                              {"message", "an error message"}}
                   .dump()
            << std::endl;

  return gcf::HttpResponse{}
      .set_header("content-type", "text/plain")
      .set_payload("Hello World!");
}
// [END functions_helloworld_error]
