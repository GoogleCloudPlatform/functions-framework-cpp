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

#include "google/cloud/functions/internal/setenv.h"
// We need _putenv_s() on WIN32 and setenv()/unsetenv() on Posix. clang-tidy
// recommends including <cstdlib>. That seems wrong, as <cstdlib> is not
// guaranteed to define the Posix/WIN32 functions.
#include <stdlib.h>  // NOLINT(modernize-deprecated-headers)

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {

namespace {
void UnsetEnv(char const* variable) {
#ifdef _WIN32
  (void)_putenv_s(variable, "");
#else
  unsetenv(variable);
#endif  // _WIN32
}

void SetEnv(char const* variable, char const* value) {
#ifdef _WIN32
  (void)_putenv_s(variable, value);
#else
  (void)setenv(variable, value, 1);
#endif  // _WIN32
}

}  // namespace

void SetEnv(std::string const& variable,
            std::optional<std::string> const& value) {
  if (!value.has_value()) {
    UnsetEnv(variable.c_str());
    return;
  }
  SetEnv(variable.c_str(), value->c_str());
}

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal
