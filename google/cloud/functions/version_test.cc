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

#include "google/cloud/functions/version.h"
#include "google/cloud/functions/internal/build_info.h"
#include <gmock/gmock.h>
#include <sstream>

namespace google::cloud::functions {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {
namespace {

using ::testing::HasSubstr;
using ::testing::Not;
using ::testing::StartsWith;

/// @test A trivial test for the Google Cloud Storage C++ Client
TEST(VersionTest, Simple) {
  EXPECT_FALSE(VersionString().empty());
  EXPECT_EQ(FUNCTIONS_FRAMEWORK_CPP_VERSION_MAJOR, VersionMajor());
  EXPECT_EQ(FUNCTIONS_FRAMEWORK_CPP_VERSION_MINOR, VersionMinor());
  EXPECT_EQ(FUNCTIONS_FRAMEWORK_CPP_VERSION_PATCH, VersionPatch());
}

/// @test Verify the version string starts with the version numbers.
TEST(VersionTest, Format) {
  std::ostringstream os;
  os << "v" << FUNCTIONS_FRAMEWORK_CPP_VERSION_MAJOR << "."
     << FUNCTIONS_FRAMEWORK_CPP_VERSION_MINOR << "."
     << FUNCTIONS_FRAMEWORK_CPP_VERSION_PATCH;
  EXPECT_THAT(VersionString(), StartsWith(os.str()));
}

/// @test Verify the version does not contain build info for release builds.
TEST(VersionTest, NoBuildInfoInRelease) {
  if (!google::cloud::functions_internal::BuildMetadata().empty()) {
    EXPECT_THAT(
        VersionString(),
        HasSubstr("+" + google::cloud::functions_internal::BuildMetadata()));
    return;
  }
  EXPECT_THAT(VersionString(), Not(HasSubstr("+")));
}

}  // namespace
}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions
