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
#include <cppcodec/base64_rfc4648.hpp>
#include <gmock/gmock.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <iterator>

namespace google::cloud::functions_internal {
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_BEGIN
namespace {

using ::testing::ElementsAre;
using ::testing::IsEmpty;

TEST(ParseCloudEventJson, Basic) {
  auto constexpr kText = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234"})js";
  auto ce = ParseCloudEventJson(kText);
  EXPECT_EQ(ce.id(), "A234-1234-1234");
  EXPECT_EQ(ce.source(), "/mycontext");
  EXPECT_EQ(ce.type(), "com.example.someevent");
  EXPECT_EQ(ce.spec_version(), functions::CloudEvent::kDefaultSpecVersion);

  EXPECT_THROW(ParseCloudEventJson("{"), std::exception);
}

TEST(ParseCloudEventJson, WithSpecVersion) {
  auto constexpr kText = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "specversion" : "1.1"})js";
  auto ce = ParseCloudEventJson(kText);
  EXPECT_EQ(ce.id(), "A234-1234-1234");
  EXPECT_EQ(ce.source(), "/mycontext");
  EXPECT_EQ(ce.type(), "com.example.someevent");
  EXPECT_EQ(ce.spec_version(), "1.1");
}

TEST(ParseCloudEventJson, MissingRequiredField) {
  auto constexpr kMissingId = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext"})js";
  auto constexpr kMissingType = R"js({
    "source" : "/mycontext",
    "id" : "A234-1234-1234"})js";
  auto constexpr kMissingSource = R"js({
    "type" : "com.example.someevent",
    "id" : "A234-1234-1234"})js";
  EXPECT_THROW(ParseCloudEventJson(kMissingId), std::exception);
  EXPECT_THROW(ParseCloudEventJson(kMissingType), std::exception);
  EXPECT_THROW(ParseCloudEventJson(kMissingSource), std::exception);
}

TEST(ParseCloudEventJson, InvalidRequiredField) {
  auto constexpr kInvalidId = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : [ "A234-1234-1234" ]})js";
  auto constexpr kInvalidType = R"js({
    "type" : [ "com.example.someevent" ],
    "source" : "/mycontext",
    "id" : "A234-1234-1234"})js";
  auto constexpr kInvalidSource = R"js({
    "type" : "com.example.someevent",
    "source" : [ "/mycontext" ],
    "id" : "A234-1234-1234"})js";
  EXPECT_THROW(ParseCloudEventJson(kInvalidId), std::exception);
  EXPECT_THROW(ParseCloudEventJson(kInvalidType), std::exception);
  EXPECT_THROW(ParseCloudEventJson(kInvalidSource), std::exception);
}

TEST(ParseCloudEventJson, WithDataContentType) {
  auto constexpr kText = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "datacontenttype" : "application/json"})js";
  auto constexpr kTextInvalid = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "datacontenttype" : {"foo": "bar"}})js";
  auto const ce = ParseCloudEventJson(kText);
  EXPECT_THAT(ce.data_content_type().value_or(""), "application/json");
  EXPECT_THROW(ParseCloudEventJson(kTextInvalid), std::exception);
}

TEST(ParseCloudEventJson, WithDataSchema) {
  auto constexpr kText = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "dataschema" : "test-schema"})js";
  auto constexpr kTextInvalid = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "dataschema" : {"foo": "bar"}})js";
  auto const ce = ParseCloudEventJson(kText);
  EXPECT_THAT(ce.data_schema().value_or(""), "test-schema");
  EXPECT_THROW(ParseCloudEventJson(kTextInvalid), std::exception);
}

TEST(ParseCloudEventJson, WithSubject) {
  auto constexpr kText = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "subject" : "test-subject"})js";
  auto constexpr kTextInvalid = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "subject" : {"foo": "bar"}})js";
  auto const ce = ParseCloudEventJson(kText);
  EXPECT_THAT(ce.subject().value_or(""), "test-subject");
  EXPECT_THROW(ParseCloudEventJson(kTextInvalid), std::exception);
}

TEST(ParseCloudEventJson, WithTime) {
  auto constexpr kText = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "time" : "2018-04-05T17:31:05Z"})js";
  auto constexpr kTextInvalid = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "time" : {"foo": "bar"}})js";
  // obtained using: date -u --date='2018-04-05T17:31:05Z' +%s
  auto constexpr kExpectedTime = std::chrono::seconds(1522949465L);
  auto const ce = ParseCloudEventJson(kText);
  auto tp = ce.time().value_or(std::chrono::system_clock::from_time_t(0));
  // This assumes the platform uses a unix epoch, reasonably safe assumption for
  // our needs.
  EXPECT_EQ(tp, std::chrono::system_clock::from_time_t(kExpectedTime.count()));
  EXPECT_THROW(ParseCloudEventJson(kTextInvalid), std::exception);
}

TEST(ParseCloudEventJson, WithData) {
  auto constexpr kText = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "data" : "some text"})js";
  auto constexpr kTextInvalid = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "dataschema" : {"foo": "bar"}})js";
  auto const ce = ParseCloudEventJson(kText);
  EXPECT_EQ(ce.data().value_or(""), "some text");
  EXPECT_THROW(ParseCloudEventJson(kTextInvalid), std::exception);
}

TEST(ParseCloudEventJson, WithDataBase64) {
  // Obtained magic string using:
  //   echo "some text" | openssl base64 -e
  auto constexpr kText = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "data_base64" : "c29tZSB0ZXh0Cg=="})js";
  auto constexpr kTextInvalid = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "dataschema" : {"foo": "bar"}})js";
  auto const ce = ParseCloudEventJson(kText);
  EXPECT_EQ(ce.data().value_or(""), "some text\n");
  EXPECT_THROW(ParseCloudEventJson(kTextInvalid), std::exception);
}

TEST(ParseCloudEventJson, Batch) {
  auto constexpr kText = R"js([
  {
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234-0"
  },
  {
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234-1"
  },
  {
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234-2"
  }
  ])js";
  auto const events = ParseCloudEventJsonBatch(kText);
  std::vector<std::string> ids;
  std::transform(events.begin(), events.end(), std::back_inserter(ids),
                 [](auto ce) { return ce.id(); });
  EXPECT_THAT(ids, ElementsAre("A234-1234-1234-0", "A234-1234-1234-1",
                               "A234-1234-1234-2"));
}

TEST(ParseCloudEventJson, BatchEmpty) {
  auto constexpr kText = R"js([])js";
  auto const events = ParseCloudEventJsonBatch(kText);
  EXPECT_THAT(events, IsEmpty());
}

TEST(ParseCloudEventJson, BatchInvalid) {
  EXPECT_THROW(ParseCloudEventJsonBatch("{"), std::exception);
  EXPECT_THROW(ParseCloudEventJsonBatch(R"js({ "foo" : " bar" })js"),
               std::exception);
}

TEST(ParseCloudEventJson, EmulateStorage) {
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
          {"data", data_base64},
      },
  }};

  auto event = nlohmann::json{
      {"specversion", "1.0"},
      {"type", "google.cloud.pubsub.topic.v1.messagePublished"},
      {"source",
       "//pubsub.googleapis.com/projects/sample-project/topics/storage"},
      {"id", "aaaaaa-1111-bbbb-2222-cccccccccccc"},
      {"time", "2020-09-29T11:32:00.000Z"},
      {"datacontenttype", "application/json"},
      {"data", payload},
  };

  auto const ce = ParseCloudEventJson(event.dump());
  EXPECT_EQ(ce.type(), "google.cloud.storage.object.v1.finalized");
  ASSERT_EQ(ce.data_content_type().value_or(""), "application/json");
  auto const actual_storage_data =
      nlohmann::json::parse(ce.data().value_or("{}"));
  auto const delta = nlohmann::json::diff(data, actual_storage_data);
  EXPECT_EQ(data, actual_storage_data) << "delta=" << delta;
}

}  // namespace
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_END
}  // namespace google::cloud::functions_internal
