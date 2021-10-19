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
#include "google/cloud/functions/internal/parse_cloud_event_legacy.h"
#include "google/cloud/functions/internal/parse_cloud_event_storage.h"

namespace google::cloud::functions_internal {
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_BEGIN

bool HasHeader(BeastRequest const& request, std::string_view header) {
  return request.count(header) != 0;
}

bool HasMinimalCloudEventHeaders(BeastRequest const& request) {
  char const* minimal_headers[] = {"ce-id", "ce-source", "ce-type"};
  return std::all_of(std::begin(minimal_headers), std::end(minimal_headers),
                     [&request](auto h) { return HasHeader(request, h); });
}

functions::CloudEvent ParseCloudEventHttpBinary(BeastRequest const& request) {
  std::string spec_version = functions::CloudEvent::kDefaultSpecVersion;
  if (HasHeader(request, "ce-specversion")) {
    spec_version = request["ce-specversion"];
  }
  functions::CloudEvent event(std::string(request.at("ce-id")),
                              std::string(request.at("ce-source")),
                              std::string(request.at("ce-type")), spec_version);
  if (HasHeader(request, "ce-datacontenttype")) {
    if (HasHeader(request, "Content-Type") &&
        request["ce-datacontenttype"] != request["Content-Type"]) {
      throw std::invalid_argument(
          "Mismatched ce-datacontentype and Content-Type header values");
    }
    event.set_data_content_type(std::string(request["ce-datacontenttype"]));
  } else if (HasHeader(request, "Content-Type")) {
    event.set_data_content_type(std::string(request["Content-Type"]));
  }

  if (HasHeader(request, "ce-dataschema")) {
    event.set_data_schema(std::string(request["ce-dataschema"]));
  }
  if (HasHeader(request, "ce-subject")) {
    event.set_subject(std::string(request["ce-subject"]));
  }
  if (HasHeader(request, "ce-time")) {
    event.set_time(std::string(request["ce-time"]));
  }

  if (request.has_content_length()) {
    event.set_data(request.body());
  }

  return event;
}

std::vector<functions::CloudEvent> ParseCloudEventHttp(
    BeastRequest const& request) {
  if (!HasHeader(request, "content-type")) {
    return {ParseCloudEventStorage(ParseCloudEventHttpBinary(request))};
  }
  auto const content_type = request["content-type"];
  if (content_type.rfind("application/cloudevents-batch+json", 0) == 0) {
    return ParseCloudEventJsonBatch(request.body());
  }
  if (content_type.rfind("application/cloudevents+json", 0) == 0) {
    return {ParseCloudEventJson(request.body())};
  }
  if (content_type.rfind("application/json", 0) == 0 &&
      !HasMinimalCloudEventHeaders(request)) {
    return {ParseCloudEventLegacy(request.body())};
  }
  return {ParseCloudEventStorage(ParseCloudEventHttpBinary(request))};
}

FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_END
}  // namespace google::cloud::functions_internal
