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
#include "google/cloud/functions/http_response.h"
#include <gmock/gmock.h>

namespace gcf = ::google::cloud::functions;
extern gcf::HttpResponse hello_world_content(gcf::HttpRequest request);
extern gcf::HttpResponse hello_world_get(gcf::HttpRequest request);
extern gcf::HttpResponse hello_world_http(gcf::HttpRequest request);
extern gcf::HttpResponse http_cors(gcf::HttpRequest request);
extern gcf::HttpResponse http_cors_auth(gcf::HttpRequest request);
extern gcf::HttpResponse http_method(gcf::HttpRequest request);
extern gcf::HttpResponse http_xml(gcf::HttpRequest request);

namespace {

TEST(ExamplesSiteTest, HelloWorldContent) {
  auto make_request = [](std::string content_type, std::string payload) {
    return gcf::HttpRequest{}
        .set_payload(std::move(payload))
        .add_header("content-type", std::move(content_type));
  };

  auto actual = hello_world_content(
      make_request("application/json", R"js({ "name": "Foo" })js"));
  EXPECT_THAT(actual.payload(), "Hello Foo");

  actual = hello_world_content(make_request("text/plain", "Bar"));
  EXPECT_THAT(actual.payload(), "Hello Bar");

  actual = hello_world_content(make_request("application/x-www-form-urlencoded",
                                            "id=1&name=Baz%20Qux&value=x"));
  EXPECT_THAT(actual.payload(), "Hello Baz Qux");

  actual = hello_world_content(make_request("application/x-www-form-urlencoded",
                                            "id=1&name=Baz%Qux&value=x"));
  EXPECT_THAT(actual.payload(), "Hello Baz%Qux");
}

TEST(ExamplesSiteTest, HelloWorldGet) {
  auto actual = hello_world_get(gcf::HttpRequest{});
  EXPECT_EQ(actual.payload(), "Hello World!");
}

TEST(ExamplesSiteTest, HelloWorlHttp) {
  auto actual = hello_world_http(
      gcf::HttpRequest{}.set_payload(R"js({ "name": "Foo" })js"));
  EXPECT_EQ(actual.payload(), "Hello Foo!");

  actual = hello_world_http(
      gcf::HttpRequest{}.set_payload(R"js({ "unused": 7 })js"));
  EXPECT_EQ(actual.payload(), "Hello World!");

  actual = hello_world_http(gcf::HttpRequest{}.set_payload("Bar"));
  EXPECT_EQ(actual.payload(), "Hello World!");
}

TEST(ExamplesSiteTest, HttpCors) {
  auto actual = http_cors(gcf::HttpRequest{}.set_verb("OPTIONS"));
  EXPECT_EQ(actual.headers().at("Access-Control-Allow-Methods"), "GET");

  actual = http_cors(gcf::HttpRequest{}.set_verb("GET"));
  EXPECT_EQ(actual.headers().at("Access-Control-Allow-Origin"), "*");
  EXPECT_EQ(actual.payload(), "Hello World!");
}

TEST(ExamplesSiteTest, HttpCorsAuth) {
  auto actual = http_cors_auth(gcf::HttpRequest{}.set_verb("OPTIONS"));
  EXPECT_EQ(actual.headers().at("Access-Control-Allow-Headers"),
            "Authorization");

  actual = http_cors_auth(gcf::HttpRequest{}.set_verb("GET"));
  EXPECT_EQ(actual.headers().at("Access-Control-Allow-Origin"),
            "https://mydomain.com");
  EXPECT_EQ(actual.headers().at("Access-Control-Allow-Credentials"), "true");
  EXPECT_EQ(actual.payload(), "Hello World!");
}

TEST(ExamplesSiteTest, HttpMethod) {
  struct Test {
    std::string verb;
    int result;
  } tests[] = {
      {"GET", gcf::HttpResponse::kOkay},
      {"POST", gcf::HttpResponse::kForbidden},
      {"PUT", gcf::HttpResponse::kMethodNotAllowed},
  };

  for (auto const& test : tests) {
    auto actual = http_method(gcf::HttpRequest{}.set_verb(test.verb));
    EXPECT_EQ(actual.result(), test.result);
  }
}

TEST(ExamplesSiteTest, HelloWorldXml) {
  auto make_request = [](std::string payload) {
    return gcf::HttpRequest{}.set_payload(std::move(payload));
  };

  auto actual = http_xml(make_request(R"xml(
<name>Foo</name>
<unused1>Bar</unused1>
<unused2>Baz</unused2>
)xml"));
  EXPECT_EQ(actual.payload(), "Hello Foo");

  actual = http_xml(make_request(R"xml(
<unused>Foo</unused>
<unused1>Bar</unused1>
<unused2>Baz</unused2>
)xml"));
  EXPECT_EQ(actual.payload(), "Hello World");
}

}  // namespace
