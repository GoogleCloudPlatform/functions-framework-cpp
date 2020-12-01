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

#ifndef FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_INTERNAL_PARSE_CLOUD_EVENT_HTTP_H
#define FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_INTERNAL_PARSE_CLOUD_EVENT_HTTP_H

#include "google/cloud/functions/internal/http_message_types.h"
#include "google/cloud/functions/cloud_event.h"
#include "google/cloud/functions/version.h"
#include <vector>

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {

/// Parse @p request as a Cloud Event, assuming the content type is binary.
functions::CloudEvent ParseCloudEventHttpBinary(BeastRequest const& request);

/// Parse @p request as a Cloud Event, using the request content type.
std::vector<functions::CloudEvent> ParseCloudEventHttp(
    BeastRequest const& request);

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal

#endif  // FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_INTERNAL_PARSE_CLOUD_EVENT_HTTP_H
