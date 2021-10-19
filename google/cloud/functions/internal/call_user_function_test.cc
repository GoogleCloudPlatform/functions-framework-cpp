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

#include "google/cloud/functions/internal/call_user_function.h"
#include <gmock/gmock.h>

namespace google::cloud::functions_internal {
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_BEGIN
namespace {

using ::testing::Contains;
namespace http = ::boost::beast::http;

TEST(CallUserFunctionHttpTest, Basic) {
  auto func = [](functions::HttpRequest const& request) {
    EXPECT_EQ(request.payload(), "Hello, is there anybody out there?");
    EXPECT_EQ(request.verb(), "PUT");
    EXPECT_EQ(request.target(), "/foo/bar");
    EXPECT_THAT(request.headers(),
                Contains(std::make_pair("x-goog-test", "test-value")));
    return functions::HttpResponse{}
        .set_header("x-goog-test", "response-header")
        .set_payload("just nod if you can hear me");
  };
  BeastRequest request;
  request.target("/foo/bar");
  request.method(boost::beast::http::verb::put);
  request.body() = "Hello, is there anybody out there?";
  request.set("x-goog-test", "test-value");
  auto response = CallUserFunction(func, std::move(request));
  EXPECT_EQ(response.result_int(), functions::HttpResponse::kOkay);
  EXPECT_EQ(response.body(), "just nod if you can hear me");
  EXPECT_EQ(response["x-goog-test"], "response-header");
}

TEST(CallUserFunctionHttpTest, ReturnError) {
  auto func = [&](functions::HttpRequest const& /*request*/) {
    return functions::HttpResponse{}.set_result(
        functions::HttpResponse::kNotFound);
  };
  BeastRequest request;
  request.target("/foo/bar/not-there");
  auto response = CallUserFunction(func, std::move(request));
  EXPECT_EQ(response.result_int(), functions::HttpResponse::kNotFound);
}

functions::HttpResponse HttpAlwaysThrow(
    functions::HttpRequest const& /*request*/) {
  throw std::runtime_error("uh-oh");
}

functions::HttpResponse HttpAlwaysThrowUnknown(
    functions::HttpRequest const& /*request*/) {
  throw "uh-oh";
}

TEST(CallUserFunctionHttpTest, ReturnErrorOnStandardException) {
  BeastRequest request;
  request.target("/foo/bar/bad");
  auto response = CallUserFunction(HttpAlwaysThrow, std::move(request));
  EXPECT_EQ(response.result(), http::status::internal_server_error);
}

TEST(CallUserFunctionHttpTest, ReturnErrorOnUnknownException) {
  BeastRequest request;
  request.target("/foo/bar/bad");
  auto response = CallUserFunction(HttpAlwaysThrowUnknown, std::move(request));
  EXPECT_EQ(response.result(), http::status::internal_server_error);
}

TEST(CallUserFunctionHttpTest, InterceptRobotsTxt) {
  BeastRequest request;
  request.target("/robots.txt");
  auto response = CallUserFunction(HttpAlwaysThrow, std::move(request));
  EXPECT_EQ(response.result(), http::status::not_found);
}

TEST(CallUserFunctionHttpTest, InterceptFaviconIco) {
  BeastRequest request;
  request.target("/favicon.ico");
  auto response = CallUserFunction(HttpAlwaysThrow, std::move(request));
  EXPECT_EQ(response.result(), http::status::not_found);
}

auto TestCloudEventRequest() {
  auto constexpr kText = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234"})js";
  BeastRequest request;
  request.target("/hello");
  request.insert("content-type", "application/cloudevents+json; charset=utf-8");
  request.body() = kText;
  request.prepare_payload();
  return request;
}

TEST(CallUserFunctionCloudEventTest, Basic) {
  auto func = [](functions::CloudEvent const& event) {
    EXPECT_EQ(event.id(), "A234-1234-1234");
    EXPECT_EQ(event.source(), "/mycontext");
    EXPECT_EQ(event.type(), "com.example.someevent");
  };
  auto const request = TestCloudEventRequest();
  auto response = CallUserFunction(func, request);
  EXPECT_EQ(response.result_int(), 200);
}

void CloudEventAlwaysThrow(functions::CloudEvent const& /*event*/) {
  throw std::runtime_error("uh-oh");
}

void CloudEventAlwaysThrowUnknown(functions::CloudEvent const& /*event*/) {
  throw "uh-oh";
}

TEST(CallUserFunctionCloudEventTest, ReturnErrorOnStandardException) {
  auto const request = TestCloudEventRequest();
  auto response = CallUserFunction(CloudEventAlwaysThrow, request);
  EXPECT_EQ(response.result(), http::status::internal_server_error);
}

TEST(CallUserFunctionCloudEventTest, ReturnErrorOnUnknownException) {
  auto const request = TestCloudEventRequest();
  auto response = CallUserFunction(CloudEventAlwaysThrowUnknown, request);
  EXPECT_EQ(response.result(), http::status::internal_server_error);
}

TEST(CallUserFunctionCloudEventTest, InterceptRobotsTxt) {
  BeastRequest request;
  request.target("/robots.txt");
  auto response = CallUserFunction(CloudEventAlwaysThrow, request);
  EXPECT_EQ(response.result(), http::status::not_found);
}

TEST(CallUserFunctionCloudEventTest, InterceptFaviconIco) {
  BeastRequest request;
  request.target("/favicon.ico");
  auto response = CallUserFunction(CloudEventAlwaysThrow, request);
  EXPECT_EQ(response.result(), http::status::not_found);
}

}  // namespace
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_END
}  // namespace google::cloud::functions_internal
