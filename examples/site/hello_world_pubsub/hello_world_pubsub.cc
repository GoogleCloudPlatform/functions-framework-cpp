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

// [START functions_helloworld_pubsub]
#include <google/cloud/functions/cloud_event.h>
#include <boost/log/trivial.hpp>
#include <cppcodec/base64_rfc4648.hpp>
#include <nlohmann/json.hpp>

namespace gcf = ::google::cloud::functions;

// Though not used in this example, the event is passed by value to support
// applications that move-out its data.
void hello_world_pubsub(gcf::CloudEvent event) {  // NOLINT
  if (event.data_content_type().value_or("") != "application/json") {
    BOOST_LOG_TRIVIAL(error) << "expected application/json data";
    return;
  }
  auto const payload = nlohmann::json::parse(event.data().value_or("{}"));
  auto const name = cppcodec::base64_rfc4648::decode<std::string>(
      payload["message"]["data"].get<std::string>());
  BOOST_LOG_TRIVIAL(info) << "Hello " << (name.empty() ? "World" : name);
}
// [END functions_helloworld_pubsub]
