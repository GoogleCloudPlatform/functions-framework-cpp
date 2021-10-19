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

#include "google/cloud/functions/internal/wrap_request.h"
#include "google/cloud/functions/http_request.h"

namespace google::cloud::functions_internal {
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_BEGIN

::google::cloud::functions::HttpRequest MakeHttpRequest(BeastRequest request) {
  auto constexpr kBeastHttpVersionFactor = 10;
  auto const version = static_cast<int>(request.version());
  auto r = ::google::cloud::functions::HttpRequest{}
               .set_verb(std::string(request.method_string()))
               .set_target(std::string(request.target()))
               .set_version(version / kBeastHttpVersionFactor,
                            version % kBeastHttpVersionFactor);
  for (auto const& h : request.base()) {
    r.add_header(std::string(h.name_string()), std::string(h.value()));
  }
  r.set_payload(std::move(request).body());
  return r;
}

FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_END
}  // namespace google::cloud::functions_internal
