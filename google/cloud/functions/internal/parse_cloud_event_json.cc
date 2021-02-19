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
#include "google/cloud/functions/internal/base64_decode.h"
#include <nlohmann/json.hpp>
#include <algorithm>

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {
namespace {

functions::CloudEvent ParseCloudEventJson(nlohmann::json const& json) {
  auto event = functions::CloudEvent(
      json.at("id").get<std::string>(), json.at("source").get<std::string>(),
      json.at("type").get<std::string>(),
      json.value("specversion", functions::CloudEvent::kDefaultSpecVersion));
  if (json.count("datacontenttype") != 0) {
    event.set_data_content_type(json.at("datacontenttype").get<std::string>());
  }
  if (json.count("dataschema") != 0) {
    event.set_data_schema(json.at("dataschema").get<std::string>());
  }
  if (json.count("subject") != 0) {
    event.set_subject(json.at("subject").get<std::string>());
  }
  if (json.count("time") != 0) {
    event.set_time(json.at("time").get<std::string>());
  }
  if (json.count("data") != 0) {
    auto d = json.at("data");
    if (d.is_object()) {
      event.set_data(d.dump());
    } else {
      event.set_data(d.get<std::string>());
    }
  } else if (json.count("data_base64") != 0) {
    // TODO(#117) - consider storing as std::vector<std::uint8_t>
    event.set_data(
        Base64Decode(json.at("data_base64").get_ref<std::string const&>()));
  }

  return event;
}

}  // namespace

/// Parse @p json_string as a Cloud Event
functions::CloudEvent ParseCloudEventJson(std::string_view json_string) {
  auto json = nlohmann::json::parse(json_string);
  return ParseCloudEventJson(json);
}

std::vector<functions::CloudEvent> ParseCloudEventJsonBatch(
    std::string_view json_string) {
  auto const json = nlohmann::json::parse(json_string);
  if (!json.is_array()) {
    throw std::invalid_argument(
        "ParseCloudEventJsonBatch - the input string must be a JSON array");
  }
  std::vector<functions::CloudEvent> events;
  events.reserve(json.size());
  std::transform(
      json.begin(), json.end(), std::back_inserter(events),
      [](nlohmann::json const& j) { return ParseCloudEventJson(j); });
  return events;
}

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal
