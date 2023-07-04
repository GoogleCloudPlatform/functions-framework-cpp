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

// [START functions_concepts_after_response]
#include <google/cloud/functions/function.h>
#include <future>

namespace gcf = ::google::cloud::functions;

gcf::Function concepts_after_response() {
  return gcf::MakeFunction([](gcf::HttpRequest const&) {
    (void)std::async(std::launch::async, [] {
      // This code may fail to complete, or even fail to start at all.
      auto constexpr kIterations = 10;
      int sum = 0;
      for (int i = 0; i != kIterations; ++i) sum += i;
      return sum;
    });
    return gcf::HttpResponse{}.set_payload("Hello World!");
  });
}
// [END functions_concepts_after_response]
