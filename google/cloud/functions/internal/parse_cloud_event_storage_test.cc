// Copyright 2021 Google LLC
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

#include "google/cloud/functions/internal/parse_cloud_event_storage.h"
#include <cppcodec/base64_rfc4648.hpp>
#include <gmock/gmock.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <iterator>

namespace google::cloud::functions_internal {
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_BEGIN
namespace {

TEST(ParseCloudEventJson, EmulateStorageBase) {
  struct {
    std::string event_type;
    std::string expected_type;
  } const test_cases[] = {
      {"OBJECT_FINALIZE", "google.cloud.storage.object.v1.finalized"},
      {"OBJECT_METADATA_UPDATE",
       "google.cloud.storage.object.v1.metadataUpdated"},
      {"OBJECT_DELETE", "google.cloud.storage.object.v1.deleted"},
      {"OBJECT_ARCHIVE", "google.cloud.storage.object.v1.archived"},
  };

  for (auto const& test : test_cases) {
    SCOPED_TRACE("Testing for " + test.event_type);
    auto const data = nlohmann::json::parse(R"js({
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
    })js");
    auto const data_base64 = cppcodec::base64_rfc4648::encode(data.dump());
    auto const attributes = nlohmann::json{
        {"notificationConfig",
         "projects/_/buckets/some-bucket/notificationConfigs/3"},
        {"eventType", test.event_type},
        {"payloadFormat", "JSON_API_V1"},
        {"bucketId", "some-bucket"},
        {"objectId", "folder/Test.cs"},
        {"objectGeneration", "1587627537231057"},
    };
    auto const payload = nlohmann::json{{
        "message",
        nlohmann::json{
            {"attributes", attributes},
            {"data", data_base64},
        },
    }};

    auto event = functions::CloudEvent(
        /*id=*/"aaaaaa-1111-bbbb-2222-cccccccccccc",
        /*source=*/
        "//pubsub.googleapis.com/projects/sample-project/topics/storage",
        /*type=*/"google.cloud.pubsub.topic.v1.messagePublished");
    event.set_data_content_type("application/json");
    event.set_data(payload.dump());

    auto const ce = ParseCloudEventStorage(event);
    EXPECT_EQ(ce.type(), test.expected_type);
    ASSERT_EQ(ce.data_content_type().value_or(""), "application/json");
    auto const actual_storage_data =
        nlohmann::json::parse(ce.data().value_or("{}"));
    auto const delta = nlohmann::json::diff(data, actual_storage_data);
    EXPECT_EQ(data, actual_storage_data) << "delta=" << delta;
  }
}

TEST(ParseCloudEventJson, EmulateStorageIdempotent) {
  auto const data = nlohmann::json::parse(R"js({
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
  })js");

  auto event = functions::CloudEvent(
      /*id=*/"aaaaaa-1111-bbbb-2222-cccccccccccc",
      /*source=*/
      "//pubsub.googleapis.com/projects/sample-project/topics/storage",
      /*type=*/"google.cloud.storage.object.v1.finalized");
  event.set_data_content_type("application/json");
  event.set_data(data.dump());

  auto const ce = ParseCloudEventStorage(event);
  EXPECT_EQ(ce.type(), "google.cloud.storage.object.v1.finalized");
}

TEST(ParseCloudEventJson, EmulateStorageMissingMessage) {
  auto const payload = nlohmann::json{{"foo", "bar"}};

  auto event = functions::CloudEvent(
      /*id=*/"aaaaaa-1111-bbbb-2222-cccccccccccc",
      /*source=*/
      "//pubsub.googleapis.com/projects/sample-project/topics/storage",
      /*type=*/"google.cloud.pubsub.topic.v1.messagePublished");
  event.set_data_content_type("application/json");
  event.set_data(payload.dump());

  auto const ce = ParseCloudEventStorage(event);
  EXPECT_EQ(ce.type(), "google.cloud.pubsub.topic.v1.messagePublished");
}

TEST(ParseCloudEventJson, EmulateStorageMissingAttributes) {
  auto const data = nlohmann::json::parse(R"js({
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
  })js");
  auto const data_base64 = cppcodec::base64_rfc4648::encode(data.dump());
  auto const payload = nlohmann::json{{
      "message",
      nlohmann::json{
          {"data", data_base64},
      },
  }};

  auto event = functions::CloudEvent(
      /*id=*/"aaaaaa-1111-bbbb-2222-cccccccccccc",
      /*source=*/
      "//pubsub.googleapis.com/projects/sample-project/topics/storage",
      /*type=*/"google.cloud.pubsub.topic.v1.messagePublished");
  event.set_data_content_type("application/json");
  event.set_data(payload.dump());

  auto const ce = ParseCloudEventStorage(event);
  EXPECT_EQ(ce.type(), "google.cloud.pubsub.topic.v1.messagePublished");
}

TEST(ParseCloudEventJson, EmulateStorageMissingData) {
  auto attributes = nlohmann::json{
      {"notificationConfig",
       "projects/_/buckets/some-bucket/notificationConfigs/3"},
      {"eventType", "OBJECT_FINALIZE"},
      {"payloadFormat", "JSON_API_V1"},
      {"bucketId", "some-bucket"},
      {"objectId", "folder/Test.cs"},
      {"objectGeneration", "1587627537231057"},
  };
  auto const payload = nlohmann::json{{
      "message",
      nlohmann::json{
          {"attributes", attributes},
      },
  }};

  auto event = functions::CloudEvent(
      /*id=*/"aaaaaa-1111-bbbb-2222-cccccccccccc",
      /*source=*/
      "//pubsub.googleapis.com/projects/sample-project/topics/storage",
      /*type=*/"google.cloud.pubsub.topic.v1.messagePublished");
  event.set_data_content_type("application/json");
  event.set_data(payload.dump());

  auto const ce = ParseCloudEventStorage(event);
  EXPECT_EQ(ce.type(), "google.cloud.pubsub.topic.v1.messagePublished");
}

TEST(ParseCloudEventJson, EmulateStorageMissingAttributeField) {
  struct {
    std::string field_name;
  } const test_cases[] = {
      {"notificationConfig"}, {"eventType"}, {"payloadFormat"},
      {"bucketId"},           {"objectId"},  {"objectGeneration"},
  };

  for (auto const& test : test_cases) {
    SCOPED_TRACE("Testing for " + test.field_name);
    auto const data = nlohmann::json::parse(R"js({
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
    })js");
    auto const data_base64 = cppcodec::base64_rfc4648::encode(data.dump());
    auto attributes = nlohmann::json{
        {"notificationConfig",
         "projects/_/buckets/some-bucket/notificationConfigs/3"},
        {"eventType", "OBJECT_FINALIZE"},
        {"payloadFormat", "JSON_API_V1"},
        {"bucketId", "some-bucket"},
        {"objectId", "folder/Test.cs"},
        {"objectGeneration", "1587627537231057"},
    };
    attributes.erase(test.field_name);
    auto const payload = nlohmann::json{{
        "message",
        nlohmann::json{
            {"attributes", attributes},
            {"data", data_base64},
        },
    }};

    auto event = functions::CloudEvent(
        /*id=*/"aaaaaa-1111-bbbb-2222-cccccccccccc",
        /*source=*/
        "//pubsub.googleapis.com/projects/sample-project/topics/storage",
        /*type=*/"google.cloud.pubsub.topic.v1.messagePublished");
    event.set_data_content_type("application/json");
    event.set_data(payload.dump());

    auto const ce = ParseCloudEventStorage(event);
    EXPECT_EQ(ce.type(), "google.cloud.pubsub.topic.v1.messagePublished");
  }
}

TEST(ParseCloudEventJson, EmulateStorageMissingInvalidAttributeField) {
  struct {
    std::string field_name;
  } const test_cases[] = {
      {"eventType"},
      {"payloadFormat"},
  };

  for (auto const& test : test_cases) {
    SCOPED_TRACE("Testing for " + test.field_name);
    auto const data = nlohmann::json::parse(R"js({
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
    })js");
    auto const data_base64 = cppcodec::base64_rfc4648::encode(data.dump());
    auto attributes = nlohmann::json{
        {"notificationConfig",
         "projects/_/buckets/some-bucket/notificationConfigs/3"},
        {"eventType", "OBJECT_FINALIZE"},
        {"payloadFormat", "JSON_API_V1"},
        {"bucketId", "some-bucket"},
        {"objectId", "folder/Test.cs"},
        {"objectGeneration", "1587627537231057"},
    };
    attributes[test.field_name] = "--invalid-value--";
    auto const payload = nlohmann::json{{
        "message",
        nlohmann::json{
            {"attributes", attributes},
            {"data", data_base64},
        },
    }};

    auto event = functions::CloudEvent(
        /*id=*/"aaaaaa-1111-bbbb-2222-cccccccccccc",
        /*source=*/
        "//pubsub.googleapis.com/projects/sample-project/topics/storage",
        /*type=*/"google.cloud.pubsub.topic.v1.messagePublished");
    event.set_data_content_type("application/json");
    event.set_data(payload.dump());

    auto const ce = ParseCloudEventStorage(event);
    EXPECT_EQ(ce.type(), "google.cloud.pubsub.topic.v1.messagePublished");
  }
}

}  // namespace
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_END
}  // namespace google::cloud::functions_internal
