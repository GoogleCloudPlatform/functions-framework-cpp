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

// [START functions_log_stackdriver]
#include <google/cloud/functions/cloud_event.h>
#include <cppcodec/base64_rfc4648.hpp>
#include <nlohmann/json.hpp>
#include <iostream>

namespace gcf = ::google::cloud::functions;

void log_stackdriver(gcf::CloudEvent event) {  // NOLINT
  if (event.data_content_type().value_or("") != "application/json") {
    std::cerr << "expected application/json data";
    return;
  }
  auto const payload = nlohmann::json::parse(event.data().value_or("{}"));
  auto const data = cppcodec::base64_rfc4648::decode<std::string>(
      payload["message"]["data"].get<std::string>());
  auto const log_entry = nlohmann::json::parse(data);
  std::cout << "Log entry data: " << log_entry.dump(/*indent=*/2) << "\n";
}
// [END functions_log_stackdriver]
