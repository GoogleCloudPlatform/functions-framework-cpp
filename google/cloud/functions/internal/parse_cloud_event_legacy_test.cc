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

#include "google/cloud/functions/internal/parse_cloud_event_legacy.h"
#include <absl/time/time.h>  // NOLINT
#include <gmock/gmock.h>
#include <nlohmann/json.hpp>

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {
namespace {

TEST(ParseCloudEventLegacy, Basic) {
  auto constexpr kInput = R"js({
    "data": {
      "email": "test@nowhere.com",
      "metadata": {
        "createdAt": "2020-05-26T10:42:27Z",
        "lastSignedInAt": "2020-10-24T11:00:00Z"
      },
      "providerData": [
        {
          "email": "test@nowhere.com",
          "providerId": "password",
          "uid": "test@nowhere.com"
        }
      ],
      "uid": "UUpby3s4spZre6kHsgVSPetzQ8l2"
    },
    "eventId": "aaaaaa-1111-bbbb-2222-cccccccccccc",
    "eventType": "providers/firebase.auth/eventTypes/user.create",
    "notSupported": {
    },
    "resource": "projects/my-project-id",
    "timestamp": "2020-09-29T11:32:00.000Z"
  })js";

  auto const ce = ParseCloudEventLegacy(kInput);
  EXPECT_EQ(ce.id(), "aaaaaa-1111-bbbb-2222-cccccccccccc");
  EXPECT_EQ(ce.source(),
            "//firebaseauth.googleapis.com/projects/my-project-id");
  EXPECT_EQ(ce.type(), "google.firebase.auth.user.v1.created");
  EXPECT_EQ(ce.spec_version(), "1.0");

  std::string err;
  absl::Time time;
  ASSERT_TRUE(absl::ParseTime(absl::RFC3339_full, "2020-09-29T11:32:00.000Z",
                              &time, &err));
  EXPECT_EQ(ce.time().value_or(std::chrono::system_clock::time_point{}),
            absl::ToChronoTime(time));
}

TEST(ParseCloudEventLegacy, WithContext) {
  auto constexpr kInput = R"js({
    "data": {
      "email": "test@nowhere.com",
      "metadata": {
        "createdAt": "2020-05-26T10:42:27Z",
        "lastSignedInAt": "2020-10-24T11:00:00Z"
      },
      "providerData": [
        {
          "email": "test@nowhere.com",
          "providerId": "password",
          "uid": "test@nowhere.com"
        }
      ],
      "uid": "UUpby3s4spZre6kHsgVSPetzQ8l2"
    },
    "context": {
      "eventId": "aaaaaa-1111-bbbb-2222-cccccccccccc",
      "eventType": "providers/firebase.auth/eventTypes/user.create",
      "service": "firebase.googleapis.com",
      "resource": {
        "name": "projects/my-project-id"
      },
      "timestamp": "2020-09-29T11:32:00.000Z"
    }
  })js";

  auto const ce = ParseCloudEventLegacy(kInput);
  EXPECT_EQ(ce.id(), "aaaaaa-1111-bbbb-2222-cccccccccccc");
  EXPECT_EQ(ce.source(),
            "//firebaseauth.googleapis.com/projects/my-project-id");
  EXPECT_EQ(ce.type(), "google.firebase.auth.user.v1.created");
  EXPECT_EQ(ce.spec_version(), "1.0");

  std::string err;
  absl::Time time;
  ASSERT_TRUE(absl::ParseTime(absl::RFC3339_full, "2020-09-29T11:32:00.000Z",
                              &time, &err));
  EXPECT_EQ(ce.time().value_or(std::chrono::system_clock::time_point{}),
            absl::ToChronoTime(time));
}

TEST(ParseCloudEventLegacy, PreferContext) {
  auto constexpr kInput = R"js({
    "data": {
      "email": "test@nowhere.com",
      "metadata": {
        "createdAt": "2020-05-26T10:42:27Z",
        "lastSignedInAt": "2020-10-24T11:00:00Z"
      },
      "providerData": [
        {
          "email": "test@nowhere.com",
          "providerId": "password",
          "uid": "test@nowhere.com"
        }
      ],
      "uid": "UUpby3s4spZre6kHsgVSPetzQ8l2"
    },
    "context": {
      "eventId": "aaaaaa-1111-bbbb-2222-cccccccccccc",
      "eventType": "providers/firebase.auth/eventTypes/user.create",
      "service": "firebase.googleapis.com",
      "resource": {
        "name": "projects/my-project-id"
      },
      "timestamp": "2020-09-29T11:32:00.000Z"
    },
    "eventId": "alternative-event-id-unused",
    "eventType": "providers/firebase.auth/eventTypes/user.delete",
    "resource": "alternative-resource-unused",
    "timestamp": "2021-02-03T04:05:06.789Z"
  })js";

  auto const ce = ParseCloudEventLegacy(kInput);
  EXPECT_EQ(ce.id(), "aaaaaa-1111-bbbb-2222-cccccccccccc");
  EXPECT_EQ(ce.source(),
            "//firebaseauth.googleapis.com/projects/my-project-id");
  EXPECT_EQ(ce.type(), "google.firebase.auth.user.v1.created");
  EXPECT_EQ(ce.spec_version(), "1.0");

  std::string err;
  absl::Time time;
  ASSERT_TRUE(absl::ParseTime(absl::RFC3339_full, "2020-09-29T11:32:00.000Z",
                              &time, &err));
  EXPECT_EQ(ce.time().value_or(std::chrono::system_clock::time_point{}),
            absl::ToChronoTime(time));
}

TEST(ParseCloudEventLegacy, MapEventTypePrefixToEventType) {
  struct {
    std::string gcf_event_type;
    std::string expected_service;
  } cases[] = {
      {"providers/cloud.firestore/eventTypes/document.write",
       "firestore.googleapis.com"},
      {"providers/google.firebase.analytics/eventTypes/event.log",
       "firebaseanalytics.googleapis.com"},
      {"providers/firebase.auth/eventTypes/user.delete",
       "firebaseauth.googleapis.com"},
      {"providers/google.firebase.database/eventTypes/ref.write",
       "firebasedatabase.googleapis.com"},
      {"providers/cloud.pubsub/eventTypes/topic.publish",
       "pubsub.googleapis.com"},
      {"google.storage.object.finalize", "storage.googleapis.com"},
  };

  for (auto const& test : cases) {
    SCOPED_TRACE("Testing with gcf_event_type=" + test.gcf_event_type);
    auto input_event = nlohmann::json{
        {"context",
         {{"eventType", test.gcf_event_type},
          {"eventId", "test-event-id"},
          {"resource", {{"name", "test-resource-name"}}}}},
        {"data", {{"unused", "123456"}}},
    };
    auto const ce = ParseCloudEventLegacy(input_event.dump());
    EXPECT_EQ(ce.source(),
              "//" + test.expected_service + "/test-resource-name");
    EXPECT_EQ(ce.id(), "test-event-id");
    EXPECT_EQ(ce.spec_version(), "1.0");
  }
}

TEST(ParseCloudEventLegacy, MapEventType) {
  struct {
    std::string gcf_event_type;
    std::string expected;
  } cases[] = {
      {"google.pubsub.topic.publish",
       "google.cloud.pubsub.topic.v1.messagePublished"},
      {"providers/cloud.pubsub/eventTypes/topic.publish",
       "google.cloud.pubsub.topic.v1.messagePublished"},
      {"google.storage.object.finalize",
       "google.cloud.storage.object.v1.finalized"},
      {"google.storage.object.delete",
       "google.cloud.storage.object.v1.deleted"},
      {"google.storage.object.archive",
       "google.cloud.storage.object.v1.archived"},
      {"google.storage.object.metadataUpdate",
       "google.cloud.storage.object.v1.metadataUpdated"},
      {"providers/cloud.firestore/eventTypes/document.write",
       "google.cloud.firestore.document.v1.written"},
      {"providers/cloud.firestore/eventTypes/document.create",
       "google.cloud.firestore.document.v1.created"},
      {"providers/cloud.firestore/eventTypes/document.update",
       "google.cloud.firestore.document.v1.updated"},
      {"providers/cloud.firestore/eventTypes/document.delete",
       "google.cloud.firestore.document.v1.deleted"},
      {"providers/firebase.auth/eventTypes/user.create",
       "google.firebase.auth.user.v1.created"},
      {"providers/firebase.auth/eventTypes/user.delete",
       "google.firebase.auth.user.v1.deleted"},
      {"providers/firebase.remoteConfig/remoteconfig.update",
       "google.firebase.remoteconfig.remoteConfig.v1.updated"},
      {"providers/google.firebase.analytics/eventTypes/event.log",
       "google.firebase.analytics.log.v1.written"},
      {"providers/google.firebase.database/eventTypes/ref.create",
       "google.firebase.database.ref.v1.created"},
      {"providers/google.firebase.database/eventTypes/ref.write",
       "google.firebase.database.ref.v1.written"},
      {"providers/google.firebase.database/eventTypes/ref.update",
       "google.firebase.database.ref.v1.updated"},
      {"providers/google.firebase.database/eventTypes/ref.delete",
       "google.firebase.database.ref.v1.deleted"},
  };

  for (auto const& test : cases) {
    SCOPED_TRACE("Testing with gcf_event_type=" + test.gcf_event_type);
    auto input_event =
        nlohmann::json{{"context",
                        {{"eventType", test.gcf_event_type},
                         {"eventId", "test-event-id"},
                         {"resource",
                          {{"service", "firebase.googleapis.com"},
                           {"name", "test-resource-name"}}}}},
                       {"data", {{"unused", "123456"}}}};
    auto const ce = ParseCloudEventLegacy(input_event.dump());
    EXPECT_EQ(ce.type(), test.expected);
    EXPECT_EQ(ce.id(), "test-event-id");
    EXPECT_EQ(ce.source(), "//firebase.googleapis.com/test-resource-name");
    EXPECT_EQ(ce.spec_version(), "1.0");
  }
}

TEST(ParseCloudEventLegacy, MapStorage) {
  auto input_event = nlohmann::json{
      {"context",
       {{"eventType", "google.storage.object.finalize"},
        {"eventId", "test-event-id"},
        {"resource",
         {{"name",
           "projects/_/buckets/sample-bucket/objects/object-name#123456"}}}}},
      {"data", {{"unused", "123456"}}},
  };
  auto const ce = ParseCloudEventLegacy(input_event.dump());
  EXPECT_EQ(ce.source(),
            "//storage.googleapis.com/projects/_/buckets/sample-bucket");
  EXPECT_EQ(ce.subject().value_or(""), "objects/object-name");
  EXPECT_EQ(ce.id(), "test-event-id");
  EXPECT_EQ(ce.spec_version(), "1.0");
}

TEST(ParseCloudEventLegacy, MapPubSub) {
  auto input_event = nlohmann::json{
      {"context",
       {{"eventType", "providers/cloud.pubsub/eventTypes/topic.publish"},
        {"eventId", "test-event-id"}}},
      {"data", {{"unused", "1234"}}},
  };
  auto const ce = ParseCloudEventLegacy(input_event.dump());
  auto data = nlohmann::json::parse(ce.data().value_or("{}"));
  ASSERT_TRUE(data.contains("message")) << "data=" << data.dump();
  auto const expected_message = nlohmann::json{{"data", {{"unused", "1234"}}}};
  ASSERT_EQ(data["message"], expected_message);
}

}  // namespace
}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal
