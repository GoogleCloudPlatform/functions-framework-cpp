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

// [START functions_helloworld_storage]
#include <google/cloud/functions/function.h>
#include <boost/log/trivial.hpp>
#include <nlohmann/json.hpp>

namespace gcf = ::google::cloud::functions;

void hello_world_storage_impl(gcf::CloudEvent const& event) {
  if (event.data_content_type().value_or("") != "application/json") {
    BOOST_LOG_TRIVIAL(error) << "expected application/json data";
    return;
  }
  auto const payload = nlohmann::json::parse(event.data().value_or("{}"));
  BOOST_LOG_TRIVIAL(info) << "Event: " << event.id();
  BOOST_LOG_TRIVIAL(info) << "Event Type: " << event.type();
  BOOST_LOG_TRIVIAL(info) << "Bucket: " << payload.value("bucket", "");
  BOOST_LOG_TRIVIAL(info) << "Object: " << payload.value("name", "");
  BOOST_LOG_TRIVIAL(info) << "Metageneration: "
                          << payload.value("metageneration", "");
  BOOST_LOG_TRIVIAL(info) << "Created: " << payload.value("timeCreated", "");
  BOOST_LOG_TRIVIAL(info) << "Updated: " << payload.value("updated", "");
}

gcf::Function hello_world_storage() {
  return gcf::MakeFunction(hello_world_storage_impl);
}
// [END functions_helloworld_storage]
