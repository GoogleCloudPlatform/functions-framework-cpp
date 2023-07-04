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

#include "google/cloud/functions/internal/function_impl.h"
#include "google/cloud/functions/internal/http_message_types.h"
#include "google/cloud/functions/internal/parse_cloud_event_json.h"
#include "google/cloud/functions/internal/setenv.h"
#include "google/cloud/functions/cloud_event.h"
#include "google/cloud/functions/function.h"
#include "google/cloud/functions/http_request.h"
#include "google/cloud/functions/http_response.h"
#include <cppcodec/base64_rfc4648.hpp>
#include <gmock/gmock.h>
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <random>

namespace gcf = ::google::cloud::functions;
namespace gcf_internal = ::google::cloud::functions_internal;

extern gcf::Function bearer_token();
extern gcf::Function concepts_after_response();
extern gcf::Function concepts_after_timeout();
extern gcf::Function concepts_filesystem();
extern gcf::Function concepts_request();
extern gcf::Function concepts_stateless();
extern gcf::Function env_vars();
extern gcf::Function hello_world_error();
extern gcf::Function hello_world_get();
extern gcf::Function hello_world_http();
extern gcf::Function hello_world_pubsub();
extern gcf::Function hello_world_storage();
extern gcf::Function http_content();
extern gcf::Function http_cors();
extern gcf::Function http_cors_auth();
extern gcf::Function http_form_data();
extern gcf::Function http_method();
extern gcf::Function http_xml();
extern gcf::Function log_helloworld();
extern gcf::Function log_stackdriver();
extern gcf::Function pubsub_subscribe();
extern gcf::Function tips_gcp_apis();
extern gcf::Function tips_infinite_retries();
extern gcf::Function tips_lazy_globals();
extern gcf::Function tips_scopes();
extern gcf::Function tips_retry();

namespace {

using ::testing::AllOf;
using ::testing::HasSubstr;
using ::testing::IsEmpty;

auto TriggerFunctionHttp(gcf::Function const& function,
                         gcf::HttpRequest const& r) {
  gcf_internal::BeastRequest request;
  request.method_string(r.verb());
  request.target(r.target());
  request.body() = r.payload();
  for (auto const& [k, v] : r.headers()) request.insert(k, v);

  auto handler =
      gcf_internal::FunctionImpl::GetImpl(function)->GetHandler("unused");
  return handler(std::move(request));
}

auto TriggerFunctionCloudEvent(gcf::Function const& function,
                               gcf::CloudEvent const& e) {
  auto payload = nlohmann::json{
      {"specversion", e.spec_version()},
      {"id", e.id()},
      {"source", e.source()},
      {"type", e.type()},
  };
  auto if_set = [&payload](std::string_view name,
                           std::optional<std::string> const& v) {
    if (v.has_value()) payload[name] = *v;
  };
  if_set("datacontenttype", e.data_content_type());
  if_set("dataschema", e.data_schema());
  if_set("subject", e.subject());
  if (e.data_content_type().value_or("") == "application/json") {
    payload["data"] = nlohmann::json::parse(e.data().value_or("{}"));
  } else if (auto const& d = e.data(); d.has_value()) {
    payload["data"] = *d;
  }

  gcf_internal::BeastRequest request;
  request.insert("content-type", "application/cloudevents+json");
  request.body() = payload.dump();

  auto handler =
      gcf_internal::FunctionImpl::GetImpl(function)->GetHandler("unused");
  return handler(std::move(request));
}

TEST(ExamplesSiteTest, BearerToken) {
  google::cloud::functions_internal::SetEnv("TARGET_URL", std::nullopt);
  google::cloud::functions_internal::SetEnv("GOOGLE_APPLICATION_CREDENTIALS",
                                            "/dev/null");

  auto function = bearer_token();
  EXPECT_EQ(TriggerFunctionHttp(function, gcf::HttpRequest{}).result_int(),
            gcf::HttpResponse::kInternalServerError);

  google::cloud::functions_internal::SetEnv(
      "TARGET_URL",
      "https://storage.googleapis.com/storage/v1/"
      "b?project=invalid-project-name---");
  EXPECT_EQ(TriggerFunctionHttp(function, gcf::HttpRequest{}).result_int(),
            gcf::HttpResponse::kInternalServerError);

  // This is a syntactically valid JSON key file, but the key has been
  // invalidated and therefore presents no security risks.
  auto const keyfile = nlohmann::json{
      {"type", "service_account"},
      {"project_id", "invalid-project-name---"},
      {"private_key_id", "a1a111aa1111a11a11a11aa111a111a1a1111111"},
      {"private_key",
       R"""(-----BEGIN PRIVATE KEY-----
MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCltiF2oP3KJJ+S
tTc1McylY+TuAi3AdohX7mmqIjd8a3eBYDHs7FlnUrFC4CRijCr0rUqYfg2pmk4a
6TaKbQRAhWDJ7XD931g7EBvCtd8+JQBNWVKnP9ByJUaO0hWVniM50KTsWtyX3up/
fS0W2R8Cyx4yvasE8QHH8gnNGtr94iiORDC7De2BwHi/iU8FxMVJAIyDLNfyk0hN
eheYKfIDBgJV2v6VaCOGWaZyEuD0FJ6wFeLybFBwibrLIBE5Y/StCrZoVZ5LocFP
T4o8kT7bU6yonudSCyNMedYmqHj/iF8B2UN1WrYx8zvoDqZk0nxIglmEYKn/6U7U
gyETGcW9AgMBAAECggEAC231vmkpwA7JG9UYbviVmSW79UecsLzsOAZnbtbn1VLT
Pg7sup7tprD/LXHoyIxK7S/jqINvPU65iuUhgCg3Rhz8+UiBhd0pCH/arlIdiPuD
2xHpX8RIxAq6pGCsoPJ0kwkHSw8UTnxPV8ZCPSRyHV71oQHQgSl/WjNhRi6PQroB
Sqc/pS1m09cTwyKQIopBBVayRzmI2BtBxyhQp9I8t5b7PYkEZDQlbdq0j5Xipoov
9EW0+Zvkh1FGNig8IJ9Wp+SZi3rd7KLpkyKPY7BK/g0nXBkDxn019cET0SdJOHQG
DiHiv4yTRsDCHZhtEbAMKZEpku4WxtQ+JjR31l8ueQKBgQDkO2oC8gi6vQDcx/CX
Z23x2ZUyar6i0BQ8eJFAEN+IiUapEeCVazuxJSt4RjYfwSa/p117jdZGEWD0GxMC
+iAXlc5LlrrWs4MWUc0AHTgXna28/vii3ltcsI0AjWMqaybhBTTNbMFa2/fV2OX2
UimuFyBWbzVc3Zb9KAG4Y7OmJQKBgQC5324IjXPq5oH8UWZTdJPuO2cgRsvKmR/r
9zl4loRjkS7FiOMfzAgUiXfH9XCnvwXMqJpuMw2PEUjUT+OyWjJONEK4qGFJkbN5
3ykc7p5V7iPPc7Zxj4mFvJ1xjkcj+i5LY8Me+gL5mGIrJ2j8hbuv7f+PWIauyjnp
Nx/0GVFRuQKBgGNT4D1L7LSokPmFIpYh811wHliE0Fa3TDdNGZnSPhaD9/aYyy78
LkxYKuT7WY7UVvLN+gdNoVV5NsLGDa4cAV+CWPfYr5PFKGXMT/Wewcy1WOmJ5des
AgMC6zq0TdYmMBN6WpKUpEnQtbmh3eMnuvADLJWxbH3wCkg+4xDGg2bpAoGAYRNk
MGtQQzqoYNNSkfus1xuHPMA8508Z8O9pwKU795R3zQs1NAInpjI1sOVrNPD7Ymwc
W7mmNzZbxycCUL/yzg1VW4P1a6sBBYGbw1SMtWxun4ZbnuvMc2CTCh+43/1l+FHe
Mmt46kq/2rH2jwx5feTbOE6P6PINVNRJh/9BDWECgYEAsCWcH9D3cI/QDeLG1ao7
rE2NcknP8N783edM07Z/zxWsIsXhBPY3gjHVz2LDl+QHgPWhGML62M0ja/6SsJW3
YvLLIc82V7eqcVJTZtaFkuht68qu/Jn1ezbzJMJ4YXDYo1+KFi+2CAGR06QILb+I
lUtj+/nH3HDQjM4ltYfTPUg=
-----END PRIVATE KEY-----
)"""},
      {"client_email",
       "invalid-service-account@invalid-project.iam.gserviceaccount.com"},
      {"client_id", "100000000000000000001"},
      {"auth_uri", "https://accounts.google.com/o/oauth2/auth"},
      {"token_uri", "https://oauth2.googleapis.com/token"},
      {"auth_provider_x509_cert_url",
       "https://www.googleapis.com/oauth2/v1/certs"},
      {"client_x509_cert_url",
       "https://www.googleapis.com/robot/v1/metadata/x509/"
       "foo-email%40foo-project.iam.gserviceaccount.com"},
  };

  std::mt19937_64 gen(std::random_device{}());
  auto rnd = [&gen] {
    return std::to_string(std::uniform_int_distribution<std::uint64_t>()(gen));
  };
  auto const filename =
      std::filesystem::path(::testing::TempDir()) / (rnd() + '-' + rnd());
  std::ofstream(filename) << keyfile.dump() << "\n";

  google::cloud::functions_internal::SetEnv("GOOGLE_APPLICATION_CREDENTIALS",
                                            filename.string());
  // We get different errors in the CI builds vs. development workstations.
  EXPECT_NE(TriggerFunctionHttp(function, gcf::HttpRequest{}).result_int(),
            gcf::HttpResponse::kOkay);

  EXPECT_EQ(TriggerFunctionHttp(
                function, gcf::HttpRequest{}.set_target("/no-auth-header"))
                .result_int(),
            gcf::HttpResponse::kBadRequest);

  std::filesystem::remove(filename);
}

TEST(ExamplesSiteTest, ConceptsAfterResponse) {
  auto function = concepts_after_response();
  auto const actual = TriggerFunctionHttp(function, gcf::HttpRequest{});
  EXPECT_THAT(actual.body(), HasSubstr("Hello World!"));
}

TEST(ExamplesSiteTest, ConceptsAfterTimeout) {
  auto function = concepts_after_timeout();
  auto const actual =
      TriggerFunctionHttp(function, gcf::HttpRequest{}.set_verb("PUT"));
  EXPECT_THAT(actual.body(), HasSubstr("Function completed!"));
}

TEST(ExamplesSiteTest, ConceptsFilesystem) {
  auto function = concepts_filesystem();
  auto const actual = TriggerFunctionHttp(function, gcf::HttpRequest{});
  EXPECT_THAT(actual.body(), Not(IsEmpty()));
}

TEST(ExamplesSiteTest, ConceptsRequest) {
  auto function = concepts_request();
  auto const actual = TriggerFunctionHttp(function, gcf::HttpRequest{});
  EXPECT_THAT(actual.body(), HasSubstr("Received code"));
}

TEST(ExamplesSiteTest, ConceptsStateless) {
  auto function = concepts_stateless();
  auto const actual = TriggerFunctionHttp(function, gcf::HttpRequest{});
  EXPECT_THAT(actual.body(), HasSubstr("Instance execution count: "));
}

TEST(ExamplesSiteTest, EnvVars) {
  google::cloud::functions_internal::SetEnv("FOO", std::nullopt);
  auto function = env_vars();
  auto actual = TriggerFunctionHttp(function, gcf::HttpRequest{});
  EXPECT_THAT(actual.body(), AllOf(HasSubstr("FOO"), HasSubstr("not set")));

  google::cloud::functions_internal::SetEnv("FOO", "test-value");
  actual = TriggerFunctionHttp(function, gcf::HttpRequest{});
  EXPECT_THAT(actual.body(), HasSubstr("test-value"));
}

TEST(ExamplesSiteTest, HelloWorldError) {
  auto function = hello_world_error();
  auto actual = TriggerFunctionHttp(function, gcf::HttpRequest{});
  EXPECT_EQ(actual.body(), "Hello World!");

  actual = TriggerFunctionHttp(
      function, gcf::HttpRequest{}.set_target("/throw/exception"));
  EXPECT_EQ(actual.result_int(), gcf::HttpResponse::kInternalServerError);
  auto error = TriggerFunctionHttp(function,
                                   gcf::HttpRequest{}.set_target("/return500"));
  EXPECT_EQ(error.result_int(), gcf::HttpResponse::kInternalServerError);
}

TEST(ExamplesSiteTest, HelloWorldGet) {
  auto function = hello_world_get();
  auto actual = TriggerFunctionHttp(function, gcf::HttpRequest{});
  EXPECT_EQ(actual.body(), "Hello World!");
}

TEST(ExamplesSiteTest, HelloWorlHttp) {
  auto function = hello_world_http();
  auto actual = TriggerFunctionHttp(
      function, gcf::HttpRequest{}.set_payload(R"js({ "name": "Foo" })js"));
  EXPECT_EQ(actual.body(), "Hello Foo!");

  actual = TriggerFunctionHttp(
      function, gcf::HttpRequest{}.set_payload(R"js({ "unused": 7 })js"));
  EXPECT_EQ(actual.body(), "Hello World!");

  actual = TriggerFunctionHttp(function, gcf::HttpRequest{}.set_payload("Bar"));
  EXPECT_EQ(actual.body(), "Hello World!");
}

TEST(ExamplesSiteTest, HelloWorldPubSub) {
  // We need some input data, and this was available from:
  //   https://github.com/GoogleCloudPlatform/functions-framework-conformance
  auto const base = nlohmann::json::parse(R"js({
    "specversion": "1.0",
    "type": "google.cloud.pubsub.topic.v1.messagePublished",
    "source": "//pubsub.googleapis.com/projects/sample-project/topics/gcf-test",
    "id": "aaaaaa-1111-bbbb-2222-cccccccccccc",
    "time": "2020-09-29T11:32:00.000Z",
    "datacontenttype": "application/json",
    "data": {
      "subscription": "projects/sample-project/subscriptions/sample-subscription",
      "message": {
        "@type": "type.googleapis.com/google.pubsub.v1.PubsubMessage",
        "attributes": {
           "attr1":"attr1-value"
        },
        "data": ""
      }
    }
  })js");

  // Test with different values for data.message.data
  for (auto const* data : {"dGVzdCBtZXNzYWdlIDM=", "YWJjZA==", ""}) {
    auto json = base;
    json["data"]["message"]["data"] = data;
    auto response = TriggerFunctionHttp(
        hello_world_pubsub(),
        gcf::HttpRequest{}
            .set_payload(json.dump())
            .add_header("content-type", "application/cloudevents+json")
            .add_header("ce-type", "com.example.someevent")
            .add_header("ce-source", "/mycontext")
            .add_header("ce-id", "A234-1234-1234"));
    EXPECT_EQ(response.result_int(), 200);
  }

  auto constexpr kBodyDataText = R"js({
    "specversion": "1.0",
    "type": "test.invalid.invalid",
    "source": "//pubsub.googleapis.com/projects/sample-project/topics/gcf-test",
    "id": "aaaaaa-1111-bbbb-2222-cccccccccccc",
    "time": "2020-09-29T11:32:00.000Z",
    "datacontenttype": "text/plain",
    "data": "some data"
  })js";
  auto constexpr kBodyDataJson = R"js({
    "specversion": "1.0",
    "type": "google.cloud.pubsub.topic.v1.messagePublished",
    "source": "//pubsub.googleapis.com/projects/sample-project/topics/gcf-test",
    "id": "aaaaaa-1111-bbbb-2222-cccccccccccc",
    "time": "2020-09-29T11:32:00.000Z",
    "datacontenttype": "application/json",
    "data": {
      "subscription": "projects/sample-project/subscriptions/sample-subscription"
      }
    }
  })js";

  struct TestCase {
    std::string name;
    std::string body;
  } const cases[] = {
      {"text", kBodyDataText},
      {"json", kBodyDataJson},
  };

  for (auto const& [name, body] : cases) {
    SCOPED_TRACE("Testing for " + name);
    auto response = TriggerFunctionHttp(
        hello_world_pubsub(),
        gcf::HttpRequest{}
            .set_payload(body)
            .add_header("ce-type", "com.example.someevent")
            .add_header("ce-source", "/mycontext")
            .add_header("ce-id", "A234-1234-1234"));
    EXPECT_EQ(response.result_int(), 200);
  }
}

TEST(ExamplesSiteTest, HelloWorldStorage) {
  // We need some input data, and this was available from:
  //   https://github.com/GoogleCloudPlatform/functions-framework-conformance
  auto const base = nlohmann::json::parse(R"js({
    "specversion": "1.0",
    "type": "google.cloud.storage.object.v1.finalized",
    "source": "//storage.googleapis.com/projects/_/buckets/some-bucket",
    "subject": "objects/folder/Test.cs",
    "id": "aaaaaa-1111-bbbb-2222-cccccccccccc",
    "time": "2020-09-29T11:32:00.000Z",
    "datacontenttype": "application/json",
    "data": {
      "bucket": "some-bucket",
      "contentType": "text/plain",
      "crc32c": "rTVTeQ==",
      "etag": "CNHZkbuF/ugCEAE=",
      "generation": "1587627537231057",
      "id": "some-bucket/folder/Test.cs/1587627537231057",
      "kind": "storage#object",
      "md5Hash": "kF8MuJ5+CTJxvyhHS1xzRg==",
      "mediaLink": "https://www.googleapis.com/download/storage/v1/b/some-bucket/o/folder%2FTest.cs?generation=1587627537231057\u0026alt=media",
      "metageneration": "1",
      "name": "folder/Test.cs",
      "selfLink": "https://www.googleapis.com/storage/v1/b/some-bucket/o/folder/Test.cs",
      "size": "352",
      "storageClass": "MULTI_REGIONAL",
      "timeCreated": "2020-04-23T07:38:57.230Z",
      "timeStorageClassUpdated": "2020-04-23T07:38:57.230Z",
      "updated": "2020-04-23T07:38:57.230Z"
    }
  })js");

  auto constexpr kBodyDataText = R"js({
    "specversion": "1.0",
    "type": "test.invalid.invalid",
    "source": "//pubsub.googleapis.com/projects/sample-project/topics/gcf-test",
    "id": "aaaaaa-1111-bbbb-2222-cccccccccccc",
    "time": "2020-09-29T11:32:00.000Z",
    "datacontenttype": "text/plain",
    "data": "some data"
  })js";

  auto constexpr kBodyDataJson = R"js({
    "specversion": "1.0",
    "type": "test.invalid.invalid",
    "source": "//pubsub.googleapis.com/projects/sample-project/topics/gcf-test",
    "id": "aaaaaa-1111-bbbb-2222-cccccccccccc",
    "time": "2020-09-29T11:32:00.000Z",
    "datacontenttype": "application/json"
  })js";

  struct TestCase {
    std::string name;
    std::string body;
    std::string content_type;
  } const cases[] = {
      {"base", base.dump(), "application/cloudevents+json"},
      {"text", kBodyDataText, "text/plain"},
      {"json", kBodyDataJson, "application/cloudevents+json"},
  };

  for (auto const& [name, body, content_type] : cases) {
    SCOPED_TRACE("Testing for " + name);
    auto response =
        TriggerFunctionHttp(hello_world_storage(),
                            gcf::HttpRequest{}
                                .set_payload(body)
                                .add_header("content-type", content_type)
                                .add_header("ce-type", "com.example.someevent")
                                .add_header("ce-source", "/mycontext")
                                .add_header("ce-id", "A234-1234-1234"));
    EXPECT_EQ(response.result_int(), 200);
  }
}

TEST(ExamplesSiteTest, HttpContent) {
  auto make_request = [](std::string content_type, std::string payload) {
    return gcf::HttpRequest{}
        .add_header("content-type", std::move(content_type))
        .set_payload(std::move(payload));
  };

  auto function = http_content();
  auto actual = TriggerFunctionHttp(
      function, make_request("application/json", R"js({ "name": "Foo" })js"));
  EXPECT_THAT(actual.body(), "Hello Foo");

  actual = TriggerFunctionHttp(function, make_request("text/plain", "Bar"));
  EXPECT_THAT(actual.body(), "Hello Bar");

  actual = TriggerFunctionHttp(function,
                               make_request("application/x-www-form-urlencoded",
                                            "id=1&name=Baz%20Qux&value=x"));
  EXPECT_THAT(actual.body(), "Hello Baz Qux");

  actual = TriggerFunctionHttp(function,
                               make_request("application/x-www-form-urlencoded",
                                            "id=1&name=Baz%Qux&value=x"));
  EXPECT_THAT(actual.body(), "Hello Baz%Qux");
}

TEST(ExamplesSiteTest, HttpCors) {
  auto function = http_cors();
  auto actual =
      TriggerFunctionHttp(function, gcf::HttpRequest{}.set_verb("OPTIONS"));
  EXPECT_EQ(actual.at("Access-Control-Allow-Methods"), "GET");

  actual = TriggerFunctionHttp(function, gcf::HttpRequest{}.set_verb("GET"));
  EXPECT_EQ(actual.at("Access-Control-Allow-Origin"), "*");
  EXPECT_EQ(actual.body(), "Hello World!");
}

TEST(ExamplesSiteTest, HttpCorsAuth) {
  auto function = http_cors_auth();
  auto actual =
      TriggerFunctionHttp(function, gcf::HttpRequest{}.set_verb("OPTIONS"));
  EXPECT_EQ(actual.at("Access-Control-Allow-Headers"), "Authorization");

  actual = TriggerFunctionHttp(function, gcf::HttpRequest{}.set_verb("GET"));
  EXPECT_EQ(actual.at("Access-Control-Allow-Origin"), "https://mydomain.com");
  EXPECT_EQ(actual.at("Access-Control-Allow-Credentials"), "true");
  EXPECT_EQ(actual.body(), "Hello World!");
}

TEST(ExamplesSiteTest, HttpFormData) {
  auto constexpr kPayload =
      "\r\n--boundary\r\n"  //
      R"""(Content-Disposition: form-data; name="field1")"""
      "\r\n"            //
      "\r\n"            //
      "value1\r\n"      //
      "--boundary\r\n"  //
      R"""(Content-Disposition: form-data; name = "field2"; filename="example.txt")"""
      "\r\n"              //
      "\r\n"              //
      "value1\r\n"        //
      "--boundary--\r\n"  //
      ;

  // Test with both quoted and unquoted boundaries.
  auto function = http_form_data();
  for (auto const* content_type :
       {R"""(multipart/form-data;boundary="boundary")""",
        R"""(multipart/form-data;boundary=boundary)"""}) {
    SCOPED_TRACE("Testing with content_type = " + std::string(content_type));
    auto actual = TriggerFunctionHttp(
        function, gcf::HttpRequest{}
                      .add_header("content-type", content_type)
                      .set_payload(kPayload)
                      .set_verb("POST"));
    ASSERT_EQ(actual.result_int(), gcf::HttpResponse::kOkay);
    auto const actual_payload = nlohmann::json::parse(actual.body());
    auto const expected_payload = nlohmann::json{
        {"parts",
         {
             {{"bodySize", 45}, {"headerCount", 1}, {"name", "\"field1\""}},
             {{"bodySize", 71},
              {"filename", "\"example.txt\""},
              {"headerCount", 1},
              {"isFile", true}},
         }}};
    EXPECT_EQ(actual_payload, expected_payload);
  }

  EXPECT_EQ(TriggerFunctionHttp(function, gcf::HttpRequest{}).result_int(),
            gcf::HttpResponse::kMethodNotAllowed);
  EXPECT_EQ(TriggerFunctionHttp(function, gcf::HttpRequest{}.set_verb("POST"))
                .result_int(),
            gcf::HttpResponse::kBadRequest);
  EXPECT_EQ(TriggerFunctionHttp(function,
                                gcf::HttpRequest{}.set_verb("POST").add_header(
                                    "content-type", "application/json"))
                .result_int(),
            gcf::HttpResponse::kBadRequest);
  EXPECT_EQ(TriggerFunctionHttp(
                function, gcf::HttpRequest{}
                              .add_header("content-type", "multipart/form-data")
                              .set_verb("POST"))
                .result_int(),
            gcf::HttpResponse::kInternalServerError);
}

TEST(ExamplesSiteTest, HttpMethod) {
  struct Test {
    std::string verb;
    int result;
  } const tests[] = {
      {"GET", gcf::HttpResponse::kOkay},
      {"POST", gcf::HttpResponse::kForbidden},
      {"PUT", gcf::HttpResponse::kMethodNotAllowed},
  };

  auto function = http_method();
  for (auto const& test : tests) {
    auto actual =
        TriggerFunctionHttp(function, gcf::HttpRequest{}.set_verb(test.verb));
    EXPECT_EQ(actual.result_int(), test.result);
  }
}

TEST(ExamplesSiteTest, HttpXml) {
  auto make_request = [](std::string payload) {
    return gcf::HttpRequest{}.set_payload(std::move(payload));
  };

  auto function = http_xml();
  auto actual = TriggerFunctionHttp(function, make_request(R"xml(
<name>Foo</name>
<unused1>Bar</unused1>
<unused2>Baz</unused2>
)xml"));
  EXPECT_EQ(actual.body(), "Hello Foo");

  actual = TriggerFunctionHttp(function, make_request(R"xml(
<unused>Foo</unused>
<unused1>Bar</unused1>
<unused2>Baz</unused2>
)xml"));
  EXPECT_EQ(actual.body(), "Hello World");
}

TEST(ExamplesSiteTest, LogHelloWorld) {
  auto function = log_helloworld();
  auto actual = TriggerFunctionHttp(function, gcf::HttpRequest{});
  EXPECT_EQ(actual.body(), "Hello Logging!");
}

TEST(ExamplesSiteTest, LogStackdriver) {
  // We need to generate a plausible Cloud Pub/Sub message carrying a Cloud
  // Logging payload, all of that wrapped in a CloudEvent envelope. Build this
  // bottom up:
  auto const stackdriver_payload = nlohmann::json{
      {"methodName", "foo"},
      {"resourceName", "projects/sample-project/something/something-name"},
      {"authenticationInfo",
       {
           {"principalEmail", "service-account@example.com"},
           {"moreInfo", "butUnused"},
       }},
  };
  auto pubsub_message = nlohmann::json{
      {"data", cppcodec::base64_rfc4648::encode(stackdriver_payload.dump())},
      {"attributes", {{"someAttribute", "unused"}}},
  };

  // The Cloud Pub/Sub envelop is stolen from:
  //   https://github.com/GoogleCloudPlatform/functions-framework-conformance
  auto envelope = nlohmann::json{
      {"specversion", "1.0"},
      {"type", "google.cloud.pubsub.topic.v1.messagePublished"},
      {"source",
       "//pubsub.googleapis.com/projects/sample-project/topics/gcf-test"},
      {"id", "aaaaaa-1111-bbbb-2222-cccccccccccc"},
      {"datacontenttype", "application/json"},
      {"time", "2020-09-29T11:32:00.000Z"},
      {"data",
       {
           {"subscription",
            "projects/sample-project/subscriptions/sample-subscription"},
           {"message", std::move(pubsub_message)},
       }},
  };

  auto const event =
      google::cloud::functions_internal::ParseCloudEventJson(envelope.dump());
  auto function = log_stackdriver();
  EXPECT_EQ(TriggerFunctionCloudEvent(function, event).result_int(),
            gcf::HttpResponse::kOkay);

  // This is just to fix the code coverage nit.
  envelope.erase("datacontenttype");
  auto const bad =
      google::cloud::functions_internal::ParseCloudEventJson(envelope.dump());
  EXPECT_EQ(TriggerFunctionCloudEvent(function, bad).result_int(),
            gcf::HttpResponse::kOkay);
}

TEST(ExamplesSiteTest, PubsubSubscribe) {
  // We need some input data, and this was available from:
  //   https://github.com/GoogleCloudPlatform/functions-framework-conformance
  auto const base = nlohmann::json::parse(R"js({
    "specversion": "1.0",
    "type": "google.cloud.pubsub.topic.v1.messagePublished",
    "source": "//pubsub.googleapis.com/projects/sample-project/topics/gcf-test",
    "id": "aaaaaa-1111-bbbb-2222-cccccccccccc",
    "time": "2020-09-29T11:32:00.000Z",
    "datacontenttype": "application/json",
    "data": {
      "subscription": "projects/sample-project/subscriptions/sample-subscription",
      "message": {
        "@type": "type.googleapis.com/google.pubsub.v1.PubsubMessage",
        "attributes": {
           "attr1":"attr1-value"
        },
        "data": ""
      }
    }
  })js");

  // Test with different values for data.message.data
  auto function = pubsub_subscribe();
  for (auto const* data : {"dGVzdCBtZXNzYWdlIDM=", "YWJjZA==", ""}) {
    auto json = base;
    json["data"]["message"]["data"] = data;
    auto const event =
        google::cloud::functions_internal::ParseCloudEventJson(json.dump());
    EXPECT_EQ(TriggerFunctionCloudEvent(function, event).result_int(),
              gcf::HttpResponse::kOkay);
  }
}

TEST(ExamplesSiteTest, TipsLazyGlobals) {
  auto function = tips_lazy_globals();
  auto actual = TriggerFunctionHttp(function, gcf::HttpRequest{});
  EXPECT_THAT(actual.body(), HasSubstr("heavy computation"));
  actual = TriggerFunctionHttp(function, gcf::HttpRequest{});
  EXPECT_THAT(actual.body(), HasSubstr("heavy computation"));
}

TEST(ExamplesSiteTest, TipsGcpApis) {
#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
  // The test creates false positives with TSAN.
  GTEST_SKIP();
#endif  // __has_feature(thread_sanitizer)
#endif  // defined(__has_feature)
  google::cloud::functions_internal::SetEnv("GCP_PROJECT", std::nullopt);
  auto function = tips_gcp_apis();
  EXPECT_EQ(TriggerFunctionHttp(function, gcf::HttpRequest{}).result_int(),
            gcf::HttpResponse::kInternalServerError);

  google::cloud::functions_internal::SetEnv("GCP_PROJECT", "test-unused");
  EXPECT_EQ(TriggerFunctionHttp(function, gcf::HttpRequest{}).result_int(),
            gcf::HttpResponse::kInternalServerError);
  EXPECT_EQ(TriggerFunctionHttp(function, gcf::HttpRequest{}.set_payload(
                                              nlohmann::json({}).dump()))
                .result_int(),
            gcf::HttpResponse::kInternalServerError);
}

TEST(ExamplesSiteTest, TipsInfiniteRetries) {
  auto function = tips_infinite_retries();
  gcf::CloudEvent event("test-id", "test-source", "test-type");
  EXPECT_EQ(TriggerFunctionCloudEvent(function, event).result_int(),
            gcf::HttpResponse::kOkay);
  event.set_time(std::chrono::system_clock::now());
  EXPECT_EQ(TriggerFunctionCloudEvent(function, event).result_int(),
            gcf::HttpResponse::kOkay);
}

TEST(ExamplesSiteTest, TipsScopes) {
  auto function = tips_scopes();
  auto actual = TriggerFunctionHttp(function, gcf::HttpRequest{});
  EXPECT_THAT(actual.body(), HasSubstr("Global: "));
  EXPECT_THAT(actual.body(), HasSubstr("Local: "));
}

TEST(ExamplesSiteTest, TipsRetry) {
  auto function = tips_retry();
  gcf::CloudEvent event("test-id", "test-source", "test-type");
  EXPECT_EQ(TriggerFunctionCloudEvent(function, event).result_int(),
            gcf::HttpResponse::kOkay);
  event.set_data_content_type("application/json");
  EXPECT_EQ(TriggerFunctionCloudEvent(function, event).result_int(),
            gcf::HttpResponse::kOkay);
  event.set_data(nlohmann::json({{"retry", false}}).dump());
  EXPECT_EQ(TriggerFunctionCloudEvent(function, event).result_int(),
            gcf::HttpResponse::kOkay);
  event.set_data(nlohmann::json({{"retry", true}}).dump());
  EXPECT_EQ(TriggerFunctionCloudEvent(function, event).result_int(),
            gcf::HttpResponse::kInternalServerError);
}

}  // namespace
