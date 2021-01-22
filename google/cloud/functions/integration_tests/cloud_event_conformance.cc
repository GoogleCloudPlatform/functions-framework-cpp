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

#include "google/cloud/functions/framework.h"
#include <absl/time/time.h>
#include <nlohmann/json.hpp>
#include <fstream>

namespace functions = ::google::cloud::functions;
void CloudEventConformance(functions::CloudEvent const& ev) {
  // This output name is hard requirement from the conformance testing
  // framework.
  auto constexpr kOutputFilename = "function_output.json";
  nlohmann::json event{
      {"id", ev.id()},
      {"source", ev.source()},
      {"type", ev.type()},
      {"specversion", ev.spec_version()},
  };
  struct {
    std::string name;
    std::optional<std::string> value;
  } optional_fields[]{
      {"datacontenttype", ev.data_content_type()},
      {"dataschema", ev.data_schema()},
      {"subject", ev.subject()},
  };
  for (auto const& of : optional_fields) {
    if (of.value) event[of.name] = *of.value;
  }
  if (ev.time()) {
    event["time"] = absl::FormatTime(
        absl::RFC3339_full, absl::FromChrono(*ev.time()), absl::UTCTimeZone());
  }
  if (ev.data()) {
    if (ev.data_content_type().value_or("") == "application/json") {
      event["data"] = nlohmann::json::parse(*ev.data());
    } else {
      event["data"] = *ev.data();
    }
  }

  std::ofstream(kOutputFilename) << event.dump() << "\n";

  // This is here just to gracefully shutdown and collect coverage data.
  if (ev.subject() == "/quit/program/0") std::exit(0);
}

int main(int argc, char* argv[]) {
  return functions::Run(argc, argv, CloudEventConformance);
}
