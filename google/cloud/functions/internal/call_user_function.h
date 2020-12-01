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

#ifndef FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_INTERNAL_CALL_USER_FUNCTION_H
#define FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_INTERNAL_CALL_USER_FUNCTION_H

#include "google/cloud/functions/internal/http_message_types.h"
#include "google/cloud/functions/cloud_event.h"
#include "google/cloud/functions/http_request.h"
#include "google/cloud/functions/http_response.h"
#include <functional>

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {

using UserHttpFunction =
    std::function<functions::HttpResponse(functions::HttpRequest)>;

using UserCloudEventFunction = std::function<void(functions::CloudEvent)>;

BeastResponse CallUserFunction(UserHttpFunction const& function,
                               BeastRequest request);

BeastResponse CallUserFunction(UserCloudEventFunction const& function,
                               BeastRequest const& request);

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal

#endif  // FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_INTERNAL_CALL_USER_FUNCTION_H
