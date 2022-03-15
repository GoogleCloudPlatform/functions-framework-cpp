// Copyright 2022 Google LLC
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

#include "google/cloud/functions/internal/function_impl.h"
#include "google/cloud/functions/function.h"
#include <gmock/gmock.h>

namespace google::cloud::functions_internal {
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_BEGIN
namespace {

using ::testing::HasSubstr;
namespace http = ::boost::beast::http;

auto SimpleHttp(functions::HttpRequest const& request) {
  std::string payload = request.payload();
  payload += "\n";
  payload += ":target: ";
  payload += request.target();
  for (auto const& [k, v] : request.headers()) {
    payload += "\n";
    payload += k;
    payload += ": ";
    payload += v;
  }
  return functions::HttpResponse{}.set_payload(std::move(payload));
}

functions::HttpResponse AlwaysThrowHttp(
    functions::HttpRequest const& /*request*/) {
  throw std::runtime_error("testing");
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

TEST(FunctionImpl, HttpNormal) {
  auto function = functions::MakeFunction(SimpleHttp);
  auto handler = FunctionImpl::GetImpl(function)->GetHandler("unused");
  BeastRequest request;
  request.target("/test-target");
  request.insert("x-test-header", "value");
  auto response = handler(request);
  EXPECT_EQ(response.result(), http::status::ok);
  EXPECT_THAT(response.body(), HasSubstr(":target: /test-target"));
  EXPECT_THAT(response.body(), HasSubstr("x-test-header: value"));
}

TEST(FunctionImpl, HttpThrow) {
  auto function = functions::MakeFunction(AlwaysThrowHttp);
  auto handler = FunctionImpl::GetImpl(function)->GetHandler("unused");
  BeastRequest request;
  request.target("/test-target");
  auto response = handler(request);
  EXPECT_EQ(response.result(), http::status::internal_server_error);
}

TEST(FunctionImpl, CloudEventNormal) {
  auto func = [](functions::CloudEvent const& event) {
    EXPECT_EQ(event.id(), "A234-1234-1234");
    EXPECT_EQ(event.source(), "/mycontext");
    EXPECT_EQ(event.type(), "com.example.someevent");
  };
  auto function = functions::MakeFunction(func);
  auto handler = FunctionImpl::GetImpl(function)->GetHandler("unused");
  auto response = handler(TestCloudEventRequest());
  EXPECT_EQ(response.result(), http::status::ok);
}

TEST(FunctionImpl, CloudEventThrow) {
  auto func = [](functions::CloudEvent const& /*event*/) {
    throw std::runtime_error("testing");
  };
  auto function = functions::MakeFunction(func);
  auto handler = FunctionImpl::GetImpl(function)->GetHandler("unused");
  auto response = handler(TestCloudEventRequest());
  EXPECT_EQ(response.result(), http::status::internal_server_error);
}

auto MakeTestMapFunction() {
  auto a = [](functions::HttpRequest const& /*r*/) {
    return functions::HttpResponse{}.set_payload("a");
  };
  auto b = [](functions::HttpRequest const& /*r*/) {
    return functions::HttpResponse{}.set_payload("c");
  };
  auto c = [](functions::CloudEvent const& /*e*/) {
    throw std::runtime_error("testing");
  };
  return functions::MakeFunction({
      {"a", functions::MakeFunction(a)},
      {"b", functions::MakeFunction(b)},
      {"c", functions::MakeFunction(c)},
  });
}

TEST(FunctionImpl, MapNormal) {
  auto function = MakeTestMapFunction();
  auto handler = FunctionImpl::GetImpl(function)->GetHandler("a");
  auto response = handler(BeastRequest());
  EXPECT_EQ(response.body(), "a");
}

TEST(FunctionImpl, MapThrow) {
  auto function = MakeTestMapFunction();
  auto handler = FunctionImpl::GetImpl(function)->GetHandler("c");
  auto response = handler(TestCloudEventRequest());
  EXPECT_EQ(response.result(), http::status::internal_server_error);
}

TEST(FunctionImpl, MapInvalid) {
  auto function = MakeTestMapFunction();
  EXPECT_THROW((void)FunctionImpl::GetImpl(function)->GetHandler("invalid"),
               std::exception);
}

}  // namespace
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_END
}  // namespace google::cloud::functions_internal
