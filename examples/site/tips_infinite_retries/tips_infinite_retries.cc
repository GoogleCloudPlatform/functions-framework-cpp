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

// [START functions_tips_infinite_retries]
#include <google/cloud/functions/cloud_event.h>
#include <iostream>

namespace gcf = ::google::cloud::functions;

namespace {
auto constexpr kMaxAge = std::chrono::seconds(10);
}  // namespace

void tips_infinite_retries(gcf::CloudEvent event) {  // NOLINT
  using std::chrono::system_clock;
  auto const age =
      system_clock::now() - event.time().value_or(system_clock::time_point());
  auto const seconds =
      std::chrono::duration_cast<std::chrono::seconds>(age).count();

  if (age >= kMaxAge) {
    std::cout << "Dropped " << event.id() << " (age " << seconds << "s)\n";
    return;
  }
  std::cout << "Processed " << event.id() << " (age " << seconds << "s)\n";
}
// [END functions_tips_infinite_retries]
