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

#include <google/cloud/functions/http_request.h>
#include <google/cloud/functions/http_response.h>
#include <gmock/gmock.h>

namespace gcf = ::google::cloud::functions;

gcf::HttpResponse HelloGcs(gcf::HttpRequest);
gcf::HttpResponse HelloMultipleSources(gcf::HttpRequest);
gcf::HttpResponse HelloWithThirdParty(gcf::HttpRequest request);
gcf::HttpResponse HelloWorld(gcf::HttpRequest);
gcf::HttpResponse HowtoUseLegacyCode(gcf::HttpRequest);

namespace hello_from_namespace {
gcf::HttpResponse HelloWorld(gcf::HttpRequest);
}  // namespace hello_from_namespace

namespace hello_from_nested_namespace::ns0::ns1 {
gcf::HttpResponse HelloWorld(gcf::HttpRequest);
}  // namespace hello_from_nested_namespace::ns0::ns1

namespace {

using ::testing::HasSubstr;

TEST(HttpExamplesTest, HelloGcs) {
  auto const actual = HelloGcs(gcf::HttpRequest{}.set_target("/"));
  EXPECT_EQ(actual.result(), gcf::HttpResponse::kBadRequest);
}

TEST(HttpExamplesTest, HelloMultipleSources) {
  auto const actual = HelloMultipleSources(gcf::HttpRequest{});
  EXPECT_THAT(actual.payload(), HasSubstr("Hello World"));
}

TEST(HttpExamplesTest, HelloWithThirdParty) {
  auto const actual =
      HelloWithThirdParty(gcf::HttpRequest{}.set_target("/here"));
  EXPECT_THAT(actual.payload(), HasSubstr("Hello at /here"));
}

TEST(HttpExamplesTest, HelloWorld) {
  auto const actual = HelloWorld(gcf::HttpRequest{});
  EXPECT_THAT(actual.payload(), HasSubstr("Hello World"));
}

TEST(HttpExamplesTest, HelloFromNamespace) {
  auto const actual = hello_from_namespace::HelloWorld(gcf::HttpRequest{});
  EXPECT_THAT(actual.payload(), HasSubstr("C++ namespace"));
}

TEST(HttpExamplesTest, HelloFromNestedNamespace) {
  auto const actual =
      hello_from_nested_namespace::ns0::ns1::HelloWorld(gcf::HttpRequest{});
  EXPECT_THAT(actual.payload(), HasSubstr("C++ namespace"));
}

TEST(HttpExampleTest, HowtoUseLegacyCode) {
  auto const actual = HowtoUseLegacyCode(gcf::HttpRequest{});
  EXPECT_THAT(actual.payload(), HasSubstr("C++"));
}

}  // namespace
