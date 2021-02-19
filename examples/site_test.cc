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

#include "google/cloud/functions/internal/parse_cloud_event_json.h"
#include "google/cloud/functions/internal/setenv.h"
#include "google/cloud/functions/cloud_event.h"
#include "google/cloud/functions/http_request.h"
#include "google/cloud/functions/http_response.h"
#include <cppcodec/base64_rfc4648.hpp>
#include <gmock/gmock.h>
#include <nlohmann/json.hpp>

namespace gcf = ::google::cloud::functions;
extern gcf::HttpResponse http_content(gcf::HttpRequest request);
extern gcf::HttpResponse hello_world_get(gcf::HttpRequest request);
extern gcf::HttpResponse hello_world_http(gcf::HttpRequest request);
extern gcf::HttpResponse http_cors(gcf::HttpRequest request);
extern gcf::HttpResponse http_cors_auth(gcf::HttpRequest request);
extern gcf::HttpResponse http_method(gcf::HttpRequest request);
extern gcf::HttpResponse http_xml(gcf::HttpRequest request);
extern gcf::HttpResponse log_helloworld(gcf::HttpRequest request);
extern void log_stackdriver(gcf::CloudEvent event);
extern gcf::HttpResponse concepts_after_response(gcf::HttpRequest request);
extern gcf::HttpResponse concepts_after_timeout(gcf::HttpRequest request);
extern gcf::HttpResponse concepts_filesystem(gcf::HttpRequest request);
extern gcf::HttpResponse concepts_request(gcf::HttpRequest request);
extern gcf::HttpResponse concepts_stateless(gcf::HttpRequest request);
extern gcf::HttpResponse env_vars(gcf::HttpRequest request);
extern gcf::HttpResponse tips_gcp_apis(gcf::HttpRequest request);
extern void tips_infinite_retries(gcf::CloudEvent event);
extern gcf::HttpResponse tips_lazy_globals(gcf::HttpRequest request);
extern gcf::HttpResponse tips_scopes(gcf::HttpRequest request);
extern void tips_retry(gcf::CloudEvent event);
extern void hello_world_pubsub(gcf::CloudEvent event);
extern void hello_world_storage(gcf::CloudEvent event);
extern void pubsub_subscribe(gcf::CloudEvent event);

namespace {

using ::testing::AllOf;
using ::testing::HasSubstr;
using ::testing::IsEmpty;

TEST(ExamplesSiteTest, HelloWorldContent) {
  auto make_request = [](std::string content_type, std::string payload) {
    return gcf::HttpRequest{}
        .set_payload(std::move(payload))
        .add_header("content-type", std::move(content_type));
  };

  auto actual = http_content(
      make_request("application/json", R"js({ "name": "Foo" })js"));
  EXPECT_THAT(actual.payload(), "Hello Foo");

  actual = http_content(make_request("text/plain", "Bar"));
  EXPECT_THAT(actual.payload(), "Hello Bar");

  actual = http_content(make_request("application/x-www-form-urlencoded",
                                     "id=1&name=Baz%20Qux&value=x"));
  EXPECT_THAT(actual.payload(), "Hello Baz Qux");

  actual = http_content(make_request("application/x-www-form-urlencoded",
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
    EXPECT_NO_THROW(hello_world_pubsub(
        google::cloud::functions_internal::ParseCloudEventJson(json.dump())));
  }

  EXPECT_NO_THROW(hello_world_pubsub(
      google::cloud::functions_internal::ParseCloudEventJson(R"js({
    "specversion": "1.0",
    "type": "test.invalid.invalid",
    "source": "//pubsub.googleapis.com/projects/sample-project/topics/gcf-test",
    "id": "aaaaaa-1111-bbbb-2222-cccccccccccc",
    "time": "2020-09-29T11:32:00.000Z",
    "datacontenttype": "text/plain",
    "data": "some data"
  })js")));

  EXPECT_THROW(hello_world_pubsub(
                   google::cloud::functions_internal::ParseCloudEventJson(R"js({
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
  })js")),
               std::exception);
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

  EXPECT_NO_THROW(hello_world_storage(
      google::cloud::functions_internal::ParseCloudEventJson(base.dump())));

  EXPECT_NO_THROW(hello_world_storage(
      google::cloud::functions_internal::ParseCloudEventJson(R"js({
    "specversion": "1.0",
    "type": "test.invalid.invalid",
    "source": "//pubsub.googleapis.com/projects/sample-project/topics/gcf-test",
    "id": "aaaaaa-1111-bbbb-2222-cccccccccccc",
    "time": "2020-09-29T11:32:00.000Z",
    "datacontenttype": "text/plain",
    "data": "some data"
  })js")));

  EXPECT_NO_THROW(hello_world_storage(
      google::cloud::functions_internal::ParseCloudEventJson(R"js({
    "specversion": "1.0",
    "type": "test.invalid.invalid",
    "source": "//pubsub.googleapis.com/projects/sample-project/topics/gcf-test",
    "id": "aaaaaa-1111-bbbb-2222-cccccccccccc",
    "time": "2020-09-29T11:32:00.000Z",
    "datacontenttype": "application/json"
  })js")));
}

TEST(ExamplesSiteTest, LogHelloWorld) {
  auto actual = log_helloworld(gcf::HttpRequest{});
  EXPECT_EQ(actual.payload(), "Hello Logging!");
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

  EXPECT_NO_THROW(log_stackdriver(
      google::cloud::functions_internal::ParseCloudEventJson(envelope.dump())));

  // This is just to fix the code coverage nit.
  envelope.erase("datacontenttype");
  EXPECT_NO_THROW(log_stackdriver(
      google::cloud::functions_internal::ParseCloudEventJson(envelope.dump())));
}

TEST(ExamplesSiteTest, ConceptsAfterResponse) {
  auto actual = concepts_after_response(gcf::HttpRequest{});
  EXPECT_THAT(actual.payload(), HasSubstr("Hello World!"));
}

TEST(ExamplesSiteTest, ConceptsAfterTimeout) {
  auto actual = concepts_after_timeout(gcf::HttpRequest{}.set_verb("PUT"));
  EXPECT_THAT(actual.payload(), HasSubstr("Function completed!"));
}

TEST(ExamplesSiteTest, ConceptsFilesystem) {
  auto actual = concepts_filesystem(gcf::HttpRequest{});
  EXPECT_THAT(actual.payload(), Not(IsEmpty()));
}

TEST(ExamplesSiteTest, ConceptsRequest) {
  auto actual = concepts_request(gcf::HttpRequest{});
  EXPECT_THAT(actual.payload(), HasSubstr("Received code"));
}

TEST(ExamplesSiteTest, ConceptsStateless) {
  auto actual = concepts_stateless(gcf::HttpRequest{});
  EXPECT_THAT(actual.payload(), HasSubstr("Instance execution count: "));
}

TEST(ExamplesSiteTest, EnvVars) {
  google::cloud::functions_internal::SetEnv("FOO", std::nullopt);
  auto actual = env_vars(gcf::HttpRequest{});
  EXPECT_THAT(actual.payload(), AllOf(HasSubstr("FOO"), HasSubstr("not set")));

  google::cloud::functions_internal::SetEnv("FOO", "test-value");
  actual = env_vars(gcf::HttpRequest{});
  EXPECT_THAT(actual.payload(), HasSubstr("test-value"));
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
  for (auto const* data : {"dGVzdCBtZXNzYWdlIDM=", "YWJjZA==", ""}) {
    auto json = base;
    json["data"]["message"]["data"] = data;
    EXPECT_NO_THROW(pubsub_subscribe(
        google::cloud::functions_internal::ParseCloudEventJson(json.dump())));
  }
}

TEST(ExamplesSiteTest, TipsLazyGlobals) {
  auto actual = tips_lazy_globals(gcf::HttpRequest{});
  EXPECT_THAT(actual.payload(), HasSubstr("heavy computation"));
  actual = tips_lazy_globals(gcf::HttpRequest{});
  EXPECT_THAT(actual.payload(), HasSubstr("heavy computation"));
}

TEST(ExamplesSiteTest, TipsGcpApis) {
#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
  // The test creates false positives with TSAN.
  GTEST_SKIP();
#endif  // __has_feature(thread_sanitizer)
#endif  // defined(__has_feature)
  google::cloud::functions_internal::SetEnv("GCP_PROJECT", std::nullopt);
  EXPECT_THROW(tips_gcp_apis(gcf::HttpRequest{}), std::runtime_error);

  google::cloud::functions_internal::SetEnv("GCP_PROJECT", "test-unused");
  EXPECT_THROW(tips_gcp_apis(gcf::HttpRequest{}), std::exception);
  EXPECT_THROW(
      tips_gcp_apis(gcf::HttpRequest{}.set_payload(nlohmann::json({}).dump())),
      std::runtime_error);
}

TEST(ExamplesSiteTest, TipsInfiniteRetries) {
  gcf::CloudEvent event("test-id", "test-source", "test-type");
  EXPECT_NO_THROW(tips_infinite_retries(event));
  event.set_time(std::chrono::system_clock::now());
  EXPECT_NO_THROW(tips_infinite_retries(event));
}

TEST(ExamplesSiteTest, TipsScopes) {
  auto actual = tips_scopes(gcf::HttpRequest{});
  EXPECT_THAT(actual.payload(), HasSubstr("Global: "));
  EXPECT_THAT(actual.payload(), HasSubstr("Local: "));
}

TEST(ExamplesSiteTest, TipsRetry) {
  gcf::CloudEvent event("test-id", "test-source", "test-type");
  EXPECT_NO_THROW(tips_retry(event));
  event.set_data_content_type("application/json");
  EXPECT_NO_THROW(tips_retry(event));
  event.set_data(nlohmann::json({{"retry", false}}).dump());
  EXPECT_NO_THROW(tips_retry(event));
  event.set_data(nlohmann::json({{"retry", true}}).dump());
  EXPECT_THROW(tips_retry(event), std::exception);
}

}  // namespace
