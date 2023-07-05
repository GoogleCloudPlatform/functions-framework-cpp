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

// [START functions_tips_retry]
#include <google/cloud/functions/function.h>
#include <nlohmann/json.hpp>
#include <iostream>

namespace gcf = ::google::cloud::functions;

gcf::Function tips_retry() {
  return gcf::MakeFunction([](gcf::CloudEvent const& event) {
    if (event.data_content_type().value_or("") != "application/json") {
      std::cerr << "Error: expected application/json data\n";
      return;
    }
    auto const payload = nlohmann::json::parse(event.data().value_or("{}"));
    auto const retry = payload.value("retry", false);
    if (retry) throw std::runtime_error("Throwing exception to force retry");
    // Normal processing goes here.
  });
}
// [END functions_tips_retry]
