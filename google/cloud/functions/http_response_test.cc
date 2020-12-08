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

#include "google/cloud/functions/http_response.h"
#include <gmock/gmock.h>

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {
namespace {

using ::testing::ElementsAre;
using ::testing::IsEmpty;

TEST(WrapResponseTest, Payload) {
  functions::HttpResponse response;
  EXPECT_THAT(response.payload(), IsEmpty());
  auto const hello = std::string("Hello");
  response.set_payload(std::string(hello));
  EXPECT_EQ(response.payload(), hello);
  auto const goodbye = std::string("Goodbye");
}

TEST(WrapResponseTest, Result) {
  functions::HttpResponse response;
  EXPECT_EQ(response.result(), functions::HttpResponse::kOkay);
  response.set_result(functions::HttpResponse::kNotFound);
  EXPECT_EQ(response.result(), functions::HttpResponse::kNotFound);
}

TEST(WrapResponseTest, Headers) {
  functions::HttpResponse response;
  EXPECT_THAT(response.headers(), IsEmpty());
  response.set_header("Content-Type", "application/json");
  EXPECT_THAT(response.headers(),
              ElementsAre(std::make_pair("Content-Type", "application/json")));
  response.set_header("x-goog-test", "a");
  response.set_header("x-goog-test", "b");
  EXPECT_THAT(response.headers(),
              ElementsAre(std::make_pair("Content-Type", "application/json"),
                          std::make_pair("x-goog-test", "b")));
}

TEST(WrapResponseTest, Version) {
  functions::HttpResponse response;
  EXPECT_EQ(response.version_major(), 1);
  EXPECT_EQ(response.version_minor(), 1);
  response.set_version(1, 0);
  EXPECT_EQ(response.version_major(), 1);
  EXPECT_EQ(response.version_minor(), 0);
}

}  // namespace
}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal
