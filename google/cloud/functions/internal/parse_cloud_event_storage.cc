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
#include "google/cloud/functions/internal/base64_decode.h"
#include <nlohmann/json.hpp>

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {

functions::CloudEvent ParseCloudEventStorage(functions::CloudEvent e) {
  if (e.type() != "google.cloud.pubsub.topic.v1.messagePublished") return e;
  if (e.data_content_type().value_or("") != "application/json") return e;

  // If the event looks like a storage event, reparse it and return that event
  // instead.
  auto const payload = nlohmann::json::parse(e.data().value_or("{}"));
  if (payload.count("message") == 0) return e;
  auto const& message = payload.at("message");
  if (message.count("attributes") == 0 || message.count("data") == 0) return e;
  auto const& attributes = message.at("attributes");
  char const* required_attributes[] = {
      "notificationConfig", "eventType", "payloadFormat",
      "bucketId",           "objectId",  "objectGeneration",
  };
  auto const has_all_attributes = std::all_of(
      std::begin(required_attributes), std::end(required_attributes),
      [&attributes](char const* a) { return attributes.count(a) != 0; });
  if (!has_all_attributes) return e;
  if (attributes.value("payloadFormat", "") != "JSON_API_V1") return e;
  static auto const kMessageTypeMappings =
      std::unordered_map<std::string, std::string>{
          {"OBJECT_FINALIZE", "google.cloud.storage.object.v1.finalized"},
          {"OBJECT_METADATA_UPDATE",
           "google.cloud.storage.object.v1.metadataUpdated"},
          {"OBJECT_DELETE", "google.cloud.storage.object.v1.deleted"},
          {"OBJECT_ARCHIVE", "google.cloud.storage.object.v1.archived"},
      };
  auto mapped = kMessageTypeMappings.find(attributes.value("eventType", ""));
  if (mapped == kMessageTypeMappings.end()) return e;

  auto source = "//storage.googleapis.com/projects/_/buckets/" +
                attributes.value("bucketId", "");
  auto const& event_type = mapped->second;
  auto event = functions::CloudEvent(e.id(), std::move(source), event_type,
                                     e.spec_version());
  event.set_data_content_type("application/json");
  event.set_data_schema("google.events.cloud.storage.v1.StorageObjectData");
  event.set_subject("objects/" + attributes.value("objectId", ""));
  if (e.time().has_value()) event.set_time(e.time().value());
  event.set_data(Base64Decode(message.value("data", "")));

  return event;
}

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal
