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

#include "google/cloud/functions/internal/framework.h"
#include <cstdlib>
#include <cstring>
#include <sstream>

namespace functions = google::cloud::functions_internal;
using ::google::cloud::functions::HttpRequest;
using ::google::cloud::functions::HttpResponse;

HttpResponse EchoServer(HttpRequest const& request) {
  auto const& target = request.target();
  if (target == "/quit/program/0") std::exit(0);
  if (target.rfind("/exception/", 0) == 0) throw std::runtime_error(target);

  if (target == "/ok") {
    HttpResponse response;
    response.set_header("Content-Type", "text/plain");
    response.set_payload("OK");
    return response;
  }

  if (target.rfind("/error/", 0) == 0) {
    auto code = std::stoi(target.substr(std::strlen("/error/")));
    HttpResponse response;
    response.set_result(code);
    return response;
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

  HttpResponse response;
  response.set_header("Content-Type", "application/json");
  response.set_payload(std::move(payload).str());
  return response;
}

int main(int argc, char* argv[]) {
  return functions::Run(argc, argv, EchoServer);
}
