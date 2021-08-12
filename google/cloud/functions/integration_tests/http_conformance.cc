// Copyright 2021 Google LLC
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
#include <fstream>

namespace functions = ::google::cloud::functions;

std::atomic<bool> shutdown_server{false};

functions::HttpResponse HttpConformance(functions::HttpRequest const& request) {
  auto constexpr kOutputFilename = "function_output.json";
  std::ofstream(kOutputFilename) << request.payload() << "\n";
  // This is here just to gracefully shutdown and collect coverage data.
  if (request.target() == "/quit/program/0") shutdown_server = true;
  return functions::HttpResponse{};
}

int main(int argc, char* argv[]) {
  return google::cloud::functions_internal::RunForTest(
      argc, argv, HttpConformance,
      std::function<bool()>{[] { return shutdown_server.load(); }},
      std::function<void(int)>([](int) {}));
}
