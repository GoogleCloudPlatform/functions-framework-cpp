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

#include "google/cloud/functions/internal/parse_cloud_event_http.h"
#include "google/cloud/functions/internal/parse_cloud_event_json.h"
#include "google/cloud/functions/internal/parse_cloud_event_storage.h"

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {

functions::CloudEvent ParseCloudEventHttpBinary(BeastRequest const& request) {
  auto has_header = [&request](std::string_view header) {
    return request.find(header) != request.end();
  };
  std::string spec_version = functions::CloudEvent::kDefaultSpecVersion;
  if (has_header("ce-specversion")) {
    spec_version = request["ce-specversion"];
  }
  functions::CloudEvent event(std::string(request.at("ce-id")),
                              std::string(request.at("ce-source")),
                              std::string(request.at("ce-type")), spec_version);
  if (has_header("ce-datacontenttype")) {
    if (has_header("Content-Type") &&
        request["ce-datacontenttype"] != request["Content-Type"]) {
      throw std::invalid_argument(
          "Mismatched ce-datacontentype and Content-Type header values");
    }
    event.set_data_content_type(std::string(request["ce-datacontenttype"]));
  } else if (has_header("Content-Type")) {
    event.set_data_content_type(std::string(request["Content-Type"]));
  }

  if (has_header("ce-dataschema")) {
    event.set_data_schema(std::string(request["ce-dataschema"]));
  }
  if (has_header("ce-subject")) {
    event.set_subject(std::string(request["ce-subject"]));
  }
  if (has_header("ce-time")) {
    event.set_time(std::string(request["ce-time"]));
  }

  if (request.has_content_length()) {
    event.set_data(request.body());
  }

  return event;
}

std::vector<functions::CloudEvent> ParseCloudEventHttp(
    BeastRequest const& request) {
  if (request.count("content-type") == 0) {
    return {ParseCloudEventStorage(ParseCloudEventHttpBinary(request))};
  }
  auto content_type = request["content-type"];
  if (content_type.rfind("application/cloudevents-batch+json", 0) == 0) {
    return ParseCloudEventJsonBatch(request.body());
  }
  if (content_type.rfind("application/cloudevents+json", 0) == 0) {
    return {ParseCloudEventJson(request.body())};
  }
  return {ParseCloudEventStorage(ParseCloudEventHttpBinary(request))};
}

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal
