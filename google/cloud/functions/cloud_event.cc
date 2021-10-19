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

#include "google/cloud/functions/cloud_event.h"
#include <absl/time/time.h>  // NOLINT(modernize-deprecated-headers)

namespace google::cloud::functions {
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_BEGIN

void CloudEvent::set_time(std::string const& timestamp) {
  std::string err;
  absl::Time time;
  if (!absl::ParseTime(absl::RFC3339_full, timestamp, &time, &err)) {
    throw std::invalid_argument(err);
  }
  set_time(absl::ToChronoTime(time));
}

FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_END
}  // namespace google::cloud::functions
