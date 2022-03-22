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

#include <google/cloud/functions/function.h>
#include <google/cloud/functions/http_request.h>
#include <google/cloud/functions/internal/function_impl.h>
#include <gmock/gmock.h>

namespace gcf = ::google::cloud::functions;
namespace gcf_internal = ::google::cloud::functions_internal;

gcf::Function HelloGcs();
gcf::Function HelloMultipleSources();
gcf::Function HelloWithThirdParty();
gcf::Function HelloWorld();
gcf::Function HowtoUseLegacyCode();

namespace hello_from_namespace {
gcf::Function HelloWorld();
}  // namespace hello_from_namespace

namespace hello_from_nested_namespace::ns0::ns1 {
gcf::Function HelloWorld();
}  // namespace hello_from_nested_namespace::ns0::ns1

namespace {

using ::testing::HasSubstr;

auto TriggerFunction(gcf::Function const& function,
                     std::optional<std::string> target = std::nullopt) {
  gcf_internal::BeastRequest request;
  if (target) request.target(std::move(*target));

  auto handler =
      gcf_internal::FunctionImpl::GetImpl(function)->GetHandler("unused");
  return handler(std::move(request));
}

TEST(HttpExamplesTest, HelloGcs) {
  auto const actual = TriggerFunction(HelloGcs(), "/");
  EXPECT_EQ(actual.result_int(), 400);
}

TEST(HttpExamplesTest, HelloMultipleSources) {
  auto const actual = TriggerFunction(HelloMultipleSources());
  EXPECT_THAT(actual.body(), HasSubstr("Hello World"));
}

TEST(HttpExamplesTest, HelloWithThirdParty) {
  auto const actual = TriggerFunction(HelloWithThirdParty(), "/here");
  EXPECT_THAT(actual.body(), HasSubstr("Hello at /here"));
}

TEST(HttpExamplesTest, HelloWorld) {
  auto const actual = TriggerFunction(HelloWorld());
  EXPECT_THAT(actual.body(), HasSubstr("Hello World"));
}

TEST(HttpExamplesTest, HelloFromNamespace) {
  auto const actual = TriggerFunction(hello_from_namespace::HelloWorld());
  EXPECT_THAT(actual.body(), HasSubstr("C++ namespace"));
}

TEST(HttpExamplesTest, HelloFromNestedNamespace) {
  auto const actual =
      TriggerFunction(hello_from_nested_namespace::ns0::ns1::HelloWorld());
  EXPECT_THAT(actual.body(), HasSubstr("C++ namespace"));
}

TEST(HttpExampleTest, HowtoUseLegacyCode) {
  auto const actual = TriggerFunction(HowtoUseLegacyCode());
  EXPECT_THAT(actual.body(), HasSubstr("C++"));
}

}  // namespace
