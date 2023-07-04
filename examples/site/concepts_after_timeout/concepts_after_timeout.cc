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

// [START functions_concepts_after_timeout]
#include <google/cloud/functions/function.h>
#include <chrono>
#include <iostream>
#include <thread>

namespace gcf = ::google::cloud::functions;

gcf::Function concepts_after_timeout() {
  using std::chrono::minutes;
  return gcf::MakeFunction([](gcf::HttpRequest const& request) {
    std::cout << "Function running..." << std::endl;
    if (request.verb() == "GET") std::this_thread::sleep_for(minutes(2));
    std::cout << "Function completed!" << std::endl;
    return gcf::HttpResponse{}.set_payload("Function completed!");
  });
}
// [END functions_concepts_after_timeout]
