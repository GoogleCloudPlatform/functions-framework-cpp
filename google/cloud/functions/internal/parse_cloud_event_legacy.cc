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

#include "google/cloud/functions/internal/parse_cloud_event_legacy.h"
#include <nlohmann/json.hpp>
#include <deque>
#include <regex>
#include <utility>

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {
namespace {
std::string GetNestedKey(nlohmann::json const& json,
                         std::deque<std::string> path) {
  if (path.empty()) return std::string{};
  if (!json.contains(path.front())) return std::string{};
  auto const& sub = json.at(path.front());
  path.pop_front();
  if (sub.is_string() && path.empty()) return sub.get<std::string>();
  if (!sub.is_object()) return std::string{};
  return GetNestedKey(sub, std::move(path));
}

std::string GetAlternatives(nlohmann::json const& json,
                            std::deque<std::string> primary,
                            std::string const& alternative) {
  auto value = GetNestedKey(json, std::move(primary));
  if (!value.empty()) return value;
  return json.value(alternative, "");
}

std::string MapGCFTypeToService(std::string const& gcf_event_type) {
  static auto const* const kPrefixes =
      new auto(std::vector<std::pair<std::string, std::string>>{
          {"providers/cloud.firestore/", "firestore.googleapis.com"},
          {"providers/google.firebase.analytics/", "firebase.googleapis.com"},
          {"providers/firebase.auth/", "firebase.googleapis.com"},
          {"providers/google.firebase.database/", "firebase.googleapis.com"},
          {"providers/cloud.pubsub/", "pubsub.googleapis.com"},
          {"google.storage.object.", "storage.googleapis.com"},
      });
  for (auto const& [prefix, service] : *kPrefixes) {
    if (gcf_event_type.rfind(prefix, 0) == 0) return service;
  }
  throw std::runtime_error("Cannot match GCF event type <" + gcf_event_type +
                           "> to a known prefix");
}

std::string MapGCFTypeToCloudEventType(std::string const& gcf_event_type) {
  static auto const* const kMapping =
      new auto(std::map<std::string, std::string>{
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
      });
  auto p = kMapping->find(gcf_event_type);
  if (p != kMapping->end()) return p->second;
  if (gcf_event_type.rfind("google.storage.object.", 0) == 0) {
    return gcf_event_type;
  }
  throw std::runtime_error("Unknown mapping for GCF event type <" +
                           gcf_event_type + ">");
}

functions::CloudEvent ParseCloudEventLegacy(nlohmann::json const& json) {
  auto const gcf_event_type =
      GetAlternatives(json, {"context", "eventType"}, "eventType");
  auto const gcf_service = [&] {
    auto value = GetNestedKey(json, {"context", "resource", "service"});
    if (!value.empty()) return value;
    return MapGCFTypeToService(gcf_event_type);
  }();
  auto gcf_resource_name =
      GetAlternatives(json, {"context", "resource", "name"}, "resource");
  auto gcf_event_id = GetAlternatives(json, {"context", "eventId"}, "eventId");
  auto const gcf_timestamp =
      GetAlternatives(json, {"context", "timestamp"}, "timestamp");
  auto ce_type = MapGCFTypeToCloudEventType(gcf_event_type);

  auto gcf_source = "//" + gcf_service + "/" + gcf_resource_name;
  auto gcf_subject = std::string{};
  auto gcf_data = nlohmann::json{};
  if (json.contains("data")) gcf_data = json.at("data");

  if (gcf_service == "storage.googleapis.com") {
    auto const re = std::regex(
        "//storage.googleapis.com/projects/_/buckets/([^/]+)/objects/(.+)");
    std::smatch m;
    if (std::regex_match(gcf_source, m, re) && m.size() >= 2) {
      gcf_source = "//storage.googleapis.com/projects/_/buckets/" + m[1].str();
      gcf_subject = "objects/" + m[2].str();
      auto const p = gcf_subject.find_last_of('#');
      if (p != std::string::npos &&
          gcf_subject.find_first_not_of("0123456789", p + 1) ==
              std::string::npos) {
        gcf_subject = gcf_subject.substr(0, p);
      }
    }
  } else if (gcf_service == "pubsub.googleapis.com") {
    // Wrap the `data` into a message that contains the data :shrug:
    gcf_data = nlohmann::json{{"message", {{"data", std::move(gcf_data)}}}};
  }

  auto event = functions::CloudEvent(std::move(gcf_event_id),
                                     std::move(gcf_source), std::move(ce_type));
  if (!gcf_timestamp.empty()) event.set_time(gcf_timestamp);
  if (!gcf_subject.empty()) event.set_subject(std::move(gcf_subject));
  event.set_data_content_type("application/json");
  event.set_data(gcf_data.dump());
  return event;
}

}  // namespace

/// Parse @p json_string as one of the legacy GCF event formats.
functions::CloudEvent ParseCloudEventLegacy(std::string_view json_string) {
  auto json = nlohmann::json::parse(json_string);
  return ParseCloudEventLegacy(json);
}

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal