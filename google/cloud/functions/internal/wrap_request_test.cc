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

#include "google/cloud/functions/internal/wrap_request.h"
#include <gmock/gmock.h>

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {
namespace {

using ::testing::ElementsAre;

TEST(WrapRequestTest, Basic) {
  BeastRequest br;
  br.set("content-type", "application/json");
  br.body() = "Hello World\n";
  br.target("/some/random/target");
  br.method(boost::beast::http::verb::put);
  auto constexpr kHttp11 = 11;
  br.version(kHttp11);

  auto actual = FromBeast(std::move(br));
  EXPECT_EQ(actual.target(), "/some/random/target");
  EXPECT_EQ(actual.verb(), "PUT");
  EXPECT_EQ(actual.payload(), "Hello World\n");
  EXPECT_THAT(actual.headers(),
              ElementsAre(std::make_pair("content-type", "application/json")));
  EXPECT_EQ(actual.version_major(), 1);
  EXPECT_EQ(actual.version_minor(), 1);
}

}  // namespace
}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal
