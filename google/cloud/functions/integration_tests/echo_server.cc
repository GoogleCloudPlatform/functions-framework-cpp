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
#include <nlohmann/json.hpp>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <thread>

namespace functions = ::google::cloud::functions;
using functions::HttpRequest;
using functions::HttpResponse;

HttpResponse EchoServer(HttpRequest const& request) {
  auto const& target = request.target();
  if (target == "/quit/program/0") std::exit(0);
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

  auto const sleep_prefix = std::string{"/sleep/"};
  if (target.rfind(sleep_prefix, 0) == 0) {
    auto const delay = std::stoi(target.substr(sleep_prefix.size()));
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
  }

  auto tid = [] {
    std::ostringstream os;
    os << std::this_thread::get_id();
    return std::move(os).str();
  };
  auto headers = [&] {
    std::vector<std::string> h(request.headers().size());
    std::transform(request.headers().begin(), request.headers().end(),
                   h.begin(),
                   [](auto const& kv) { return kv.first + ": " + kv.second; });
    return h;
  };
  auto payload = nlohmann::json{
      {"thread", tid()},
      {"target", target},
      {"verb", request.verb()},
      {"headers", headers()},
  };

  return HttpResponse{}
      .set_header("Content-Type", "application/json")
      .set_payload(payload.dump(2));
}

int main(int argc, char* argv[]) {
  return functions::Run(argc, argv, EchoServer);
}
