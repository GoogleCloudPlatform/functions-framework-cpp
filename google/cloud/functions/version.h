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

#ifndef FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_VERSION_H
#define FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_VERSION_H

#include "google/cloud/functions/internal/version_info.h"
#include <string>

#define FUNCTIONS_FRAMEWORK_CPP_VCONCAT(Ma, Mi) v##Ma##_##Mi
#define FUNCTIONS_FRAMEWORK_CPP_VEVAL(Ma, Mi) \
  FUNCTIONS_FRAMEWORK_CPP_VCONCAT(Ma, Mi)
#define FUNCTIONS_FRAMEWORK_CPP_NS                                     \
  FUNCTIONS_FRAMEWORK_CPP_VEVAL(FUNCTIONS_FRAMEWORK_CPP_VERSION_MAJOR, \
                                FUNCTIONS_FRAMEWORK_CPP_VERSION_MINOR)

namespace google::cloud::functions {
/**
 * The C++ Functions Framework inlined, versioned namespace.
 */
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {
/// The C++ Functions Framework major version.
int constexpr VersionMajor() { return FUNCTIONS_FRAMEWORK_CPP_VERSION_MAJOR; }

/// The C++ Functions Framework minor version.
int constexpr VersionMinor() { return FUNCTIONS_FRAMEWORK_CPP_VERSION_MINOR; }

/// The C++ Functions Framework patch version.
int constexpr VersionPatch() { return FUNCTIONS_FRAMEWORK_CPP_VERSION_PATCH; }

namespace internal {
auto constexpr kMaxMinorVersions = 100;
auto constexpr kMaxPatchVersions = 100;
}  // namespace internal

/// A single integer representing the Major/Minor/Patch version.
int constexpr Version() {
  static_assert(VersionMinor() < internal::kMaxMinorVersions,
                "version_minor() should be < kMaxMinorVersions");
  static_assert(VersionPatch() < internal::kMaxPatchVersions,
                "version_patch() should be < kMaxPatchVersions");
  return internal::kMaxPatchVersions *
             (internal::kMaxMinorVersions * VersionMajor() + VersionMinor()) +
         VersionPatch();
}

/// The version as a string, in MAJOR.MINOR.PATCH+gitrev format.
std::string VersionString();

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions

#endif  // FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_VERSION_H
