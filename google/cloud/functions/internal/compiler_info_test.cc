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

#include "google/cloud/functions/internal/compiler_info.h"
#include <gmock/gmock.h>
#include <regex>

namespace google::cloud::functions_internal {
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_BEGIN
namespace {

using ::testing::Contains;

TEST(CompilerInfo, CompilerId) {
  auto const cn = CompilerId();
  EXPECT_NE(cn, "");
  EXPECT_EQ(std::string::npos,
            cn.find_first_not_of(
                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"));
}

TEST(CompilerInfo, CompilerVersion) {
  auto const cv = CompilerVersion();
  EXPECT_NE(cv, "");
  // Look for something that looks vaguely like an X.Y version number. Cannot
  // use ::testing::ContainsRegex() because that does not work when GTest is
  // compiled under MSVC.
  EXPECT_THAT(cv, Contains('.'));
  EXPECT_TRUE(std::regex_search(cv, std::regex(R"([0-9]+\.[0-9]+)")))
      << "version=" << cv;
}

TEST(CompilerInfo, LanguageVersion) {
  auto const lv = LanguageVersion();
  EXPECT_NE(lv, "");
  EXPECT_EQ(std::string::npos, lv.find_first_not_of("0123456789"));
}

}  // namespace
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_END
}  // namespace google::cloud::functions_internal
