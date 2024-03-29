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

#ifndef FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_USER_FUNCTIONS_H
#define FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_USER_FUNCTIONS_H

#include "google/cloud/functions/cloud_event.h"
#include "google/cloud/functions/http_request.h"
#include "google/cloud/functions/http_response.h"
#include "google/cloud/functions/version.h"
#include <functional>

namespace google::cloud::functions {
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_BEGIN

using UserHttpFunction =
    std::function<functions::HttpResponse(functions::HttpRequest)>;

using UserCloudEventFunction = std::function<void(functions::CloudEvent)>;

FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_END
}  // namespace google::cloud::functions

#endif  // FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_USER_FUNCTIONS_H
