// Copyright 2021 Google LLC
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

// [START functions_log_helloworld]
#include <google/cloud/functions/http_request.h>
#include <google/cloud/functions/http_response.h>
#include <nlohmann/json.hpp>
#include <iostream>

namespace gcf = ::google::cloud::functions;

gcf::HttpResponse log_helloworld(gcf::HttpRequest /*request*/) {  // NOLINT
  std::cout << "This is stdout\n";
  std::cerr << "This is stderr\n";

  std::cerr << nlohmann::json{{"message", "This has ERROR severity"},
                              {"severity", "error"}}
                   .dump()
            << "\n";
  return gcf::HttpResponse{}
      .set_header("content-type", "text/plain")
      .set_payload("Hello Logging!");
}
// [END functions_log_helloworld]
