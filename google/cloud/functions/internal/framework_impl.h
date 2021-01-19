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

#ifndef FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_INTERNAL_FRAMEWORK_IMPL_H
#define FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_INTERNAL_FRAMEWORK_IMPL_H

#include "google/cloud/functions/user_functions.h"
#include "google/cloud/functions/version.h"
#include <functional>

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {

/// Implement functions::Run(), with additional helpers for testing.
int RunForTest(int argc, char const* const argv[],
               functions::UserHttpFunction handler,
               std::function<bool()> const& shutdown,
               std::function<void(int)> const& actual_port);

/// Implement functions::Run(), with additional helpers for testing.
int RunForTest(int argc, char const* const argv[],
               functions::UserCloudEventFunction handler,
               std::function<bool()> const& shutdown,
               std::function<void(int)> const& actual_port);

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal

#endif  // FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_INTERNAL_FRAMEWORK_IMPL_H
