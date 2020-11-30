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
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <nlohmann/json.hpp>

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {

/// Parse @p json_string as a Cloud Event
functions::CloudEvent ParseCloudEventJson(std::string_view json_string) {
  auto json = nlohmann::json::parse(json_string);

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
    auto base64 = json.at("data_base64").get<std::string>();
    namespace bai = boost::archive::iterators;
    auto constexpr kBase64RawBits = 6;
    auto constexpr kBase64EncodedBits = 8;
    using Decoder =
        bai::transform_width<bai::binary_from_base64<std::string::iterator>,
                             kBase64EncodedBits, kBase64RawBits>;
    // Pad the raw string if needed.
    base64.append((4 - base64.size() % 4) % 4, '=');
    auto const last_non_pad = base64.find_last_not_of('=');
    auto const pad_count =
        std::distance(base64.begin() + last_non_pad + 1, base64.end());
    std::string data{Decoder(base64.begin()), Decoder(base64.end())};
    if (pad_count == 2) {
      data.pop_back();
      data.pop_back();
    } else if (pad_count == 1) {
      data.pop_back();
    }
    // TODO(#117) - consider storing as std::vector<std::uint8_t>
    event.set_data(std::move(data));
  }

  return event;
}

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal
