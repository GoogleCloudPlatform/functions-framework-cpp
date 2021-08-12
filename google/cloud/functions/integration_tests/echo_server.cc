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
#include <cstring>
#include <iostream>
#include <sstream>

namespace functions = ::google::cloud::functions;
using functions::HttpRequest;
using functions::HttpResponse;

std::atomic<bool> shutdown_server{false};

HttpResponse EchoServer(HttpRequest const& request) {
  auto const& target = request.target();
  if (target == "/quit/program/0") {
    shutdown_server = true;
    return HttpResponse{}
        .set_header("Content-Type", "text/plain")
        .set_payload("OK");
  }
  if (target.rfind("/exception/", 0) == 0) throw std::runtime_error(target);
  if (target.rfind("/unknown-exception/", 0) == 0) throw "uh-oh";

  if (target == "/ok") {
    return HttpResponse{}
        .set_header("Content-Type", "text/plain")
        .set_payload("OK");
  }

  if (target.rfind("/error/", 0) == 0) {
    auto code = std::stoi(target.substr(std::strlen("/error/")));
    return HttpResponse{}.set_result(code);
  }

  if (target.rfind("/buffered-stdout/", 0) == 0) {
    std::cout << "stdout: " << target << "\n";
  }

  if (target.rfind("/buffered-stderr/", 0) == 0) {
    std::clog << "stderr: " << target << "\n";
  }

  std::ostringstream payload;
  payload << "{\n"
          << R"js(  "target": ")js" << target << "\"\n"
          << R"js(  "verb": ")js" << request.verb() << "\"\n"
          << R"js(  "headers": [)js";
  for (auto [k, v] : request.headers()) {
    payload << '"' << k << ": " << v << '"' << "\n";
  }
  payload << "}\n";

  return HttpResponse{}
      .set_header("Content-Type", "application/json")
      .set_payload(std::move(payload).str());
}

int main(int argc, char* argv[]) {
  return google::cloud::functions_internal::RunForTest(
      argc, argv, EchoServer, [] { return shutdown_server.load(); },
      [](int /*port*/) {});
}
