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

#include "google/cloud/functions/internal/base64_decode.h"
#include <gmock/gmock.h>

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {
namespace {

struct TestData {
  std::string encoded_data;
  std::string expected_data;
};

TEST(Base64DecodeTest, Basic) {
  // The magic strings were obtained using:
  //   echo -n "" | openssl base64 -e
  //   echo -n "a" | openssl base64 -e
  //   echo -n "ab" | openssl base64 -e
  //   echo -n "abc" | openssl base64 -e
  TestData const test_data[] = {
      {"", ""},
      {"YQ==", "a"},
      {"YWI=", "ab"},
      {"YWJj", "abc"},
  };

  for (auto const& c : test_data) {
    auto const actual = Base64Decode(c.encoded_data);
    EXPECT_EQ(actual, c.expected_data);
  }
}

TEST(Base64DecodeTest, Underpadded) {
  TestData const test_data[] = {
      {"YQ=", "a"},
      {"YQ", "a"},
      {"YWI", "ab"},
  };
  for (auto const& c : test_data) {
    auto const actual = Base64Decode(c.encoded_data);
    EXPECT_EQ(actual, c.expected_data);
  }
}

TEST(Base64DecodeTest, Overpadded) {
  auto constexpr kExcessivePadding = "YWJj============";
  EXPECT_THROW(Base64Decode(kExcessivePadding), std::invalid_argument);
}

}  // namespace
}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal
