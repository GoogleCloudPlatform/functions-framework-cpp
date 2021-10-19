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

#include "google/cloud/functions/http_request.h"
#include <gmock/gmock.h>

namespace google::cloud::functions {
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_BEGIN
namespace {

using ::testing::ElementsAre;
using ::testing::IsEmpty;

TEST(HttpRequestTest, Default) {
  auto const actual = HttpRequest{};
  EXPECT_THAT(actual.verb(), IsEmpty());
  EXPECT_THAT(actual.target(), IsEmpty());
  EXPECT_THAT(actual.payload(), IsEmpty());
  EXPECT_EQ(actual.version_major(), 1);
  EXPECT_EQ(actual.version_minor(), 1);
}

TEST(HttpRequestTest, SimpleSetters) {
  auto const actual = HttpRequest{}
                          .set_version(1, 0)
                          .set_verb("POST")
                          .set_target("/index.html")
                          .set_payload("Hello");
  EXPECT_EQ(actual.verb(), "POST");
  EXPECT_EQ(actual.target(), "/index.html");
  EXPECT_EQ(actual.payload(), "Hello");
  EXPECT_EQ(actual.version_major(), 1);
  EXPECT_EQ(actual.version_minor(), 0);
}

TEST(HttpRequestTest, AddHeader) {
  using value_type = HttpRequest::HeadersType::value_type;

  auto const actual = HttpRequest{}
                          .add_header("x-repeated", "1")
                          .add_header("abc-header", "2")
                          .add_header("x-repeated", "2");
  EXPECT_THAT(actual.headers(), ElementsAre(value_type("abc-header", "2"),
                                            value_type("x-repeated", "1"),
                                            value_type("x-repeated", "2")));
}

TEST(HttpRequestTest, RemoveHeader) {
  using value_type = HttpRequest::HeadersType::value_type;

  auto const actual = HttpRequest{}
                          .add_header("x-repeated", "1")
                          .add_header("abc-header", "2")
                          .add_header("x-repeated", "2")
                          .remove_header("x-repeated");
  EXPECT_THAT(actual.headers(), ElementsAre(value_type("abc-header", "2")));
}

TEST(HttpRequestTest, ClearHeaders) {
  auto const actual =
      HttpRequest{}.add_header("abc-header", "2").clear_headers();
  EXPECT_THAT(actual.headers(), IsEmpty());
}

}  // namespace
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_END
}  // namespace google::cloud::functions
