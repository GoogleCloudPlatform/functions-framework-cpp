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

#include "google/cloud/functions/internal/framework_impl.h"
#include <atomic>
#include <iostream>

using ::google::cloud::functions::CloudEvent;

std::atomic<bool> shutdown_server{false};

void CloudEventHandler(CloudEvent const& event) {
  auto const& subject = event.subject().value_or("");
  if (subject == "/quit/program/0") {
    shutdown_server.store(true);
    return;
  }
  if (subject.rfind("/exception/", 0) == 0) throw std::runtime_error(subject);
  if (subject.rfind("/unknown-exception/", 0) == 0) throw "uh-oh";
  if (subject.rfind("/buffered-stdout/", 0) == 0) {
    std::cout << "stdout: " << subject << "\n";
  }
  if (subject.rfind("/buffered-stderr/", 0) == 0) {
    std::clog << "stderr: " << subject << "\n";
  }
}

int main(int argc, char* argv[]) {
  return google::cloud::functions_internal::RunForTest(
      argc, argv, CloudEventHandler, [] { return shutdown_server.load(); },
      [](int /*port*/) {});
}
