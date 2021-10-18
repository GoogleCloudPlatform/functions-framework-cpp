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

#include <google/cloud/functions/cloud_event.h>
#include <iostream>

using ::google::cloud::functions::CloudEvent;

// Though not used in this function, `event` is passed by value to support
// applications that move-out its data.
void HelloCloudEvent(CloudEvent event) {  // NOLINT
  std::cout << "Received event"
            << "\n id: " << event.id()
            << "\n subject: " << event.subject().value_or("") << std::endl;
}
