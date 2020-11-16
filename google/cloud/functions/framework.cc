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

#include "google/cloud/functions/framework.h"
#include "google/cloud/functions/internal/framework_impl.h"
#include <iostream>

namespace google::cloud::functions {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {

int Run(int argc, char const* const argv[], HttpFunction handler) noexcept try {
  return functions_internal::RunForTest(
      argc, argv, std::move(handler), [] { return false; },
      [](int /*unused*/) {});
} catch (std::exception const& ex) {
  std::cerr << "Standard C++ exception thrown " << ex.what() << "\n";
  return 1;
} catch (...) {
  std::cerr << "Unknown exception thrown\n";
  return 1;
}

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions
