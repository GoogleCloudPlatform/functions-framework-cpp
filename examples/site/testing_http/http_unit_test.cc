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

// [START functions_http_unit_test]
#include <google/cloud/functions/http_request.h>
#include <google/cloud/functions/http_response.h>
#include <gtest/gtest.h>

namespace gcf = ::google::cloud::functions;
extern gcf::HttpResponse hello_world_http_impl(gcf::HttpRequest request);

namespace {

TEST(HttpUnitTest, Success) {
  auto actual = hello_world_http_impl(
      gcf::HttpRequest{}.set_payload(R"js({ "name": "Foo" })js"));
  EXPECT_EQ(actual.payload(), "Hello Foo!");

  actual = hello_world_http_impl(
      gcf::HttpRequest{}.set_payload(R"js({ "unused": 7 })js"));
  EXPECT_EQ(actual.payload(), "Hello World!");

  actual = hello_world_http_impl(gcf::HttpRequest{}.set_payload("Bar"));
  EXPECT_EQ(actual.payload(), "Hello World!");
}

}  // namespace
// [END functions_http_unit_test]
