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

#include <google/cloud/functions/function.h>
#include <google/cloud/storage/client.h>
#include <sstream>

namespace gcf = ::google::cloud::functions;
namespace gcs = ::google::cloud::storage;

gcf::Function HelloGcs() {
  return gcf::MakeFunction([](gcf::HttpRequest const& request) {
    auto error = [] {
      return gcf::HttpResponse{}.set_result(gcf::HttpResponse::kBadRequest);
    };

    std::vector<std::string> components;
    std::istringstream split(request.target());
    for (std::string c; std::getline(split, c, '/'); components.push_back(c)) {
    }

    if (components.size() != 2) return error();
    auto const bucket = components[0];
    auto const object = components[1];
    auto client = gcs::Client::CreateDefaultClient().value();
    auto reader = client.ReadObject(bucket, object);
    std::string contents(std::istreambuf_iterator<char>{reader},
                         std::istreambuf_iterator<char>{});
    if (!reader.status().ok()) return error();

    return gcf::HttpResponse{}
        .set_header("Content-Type", "application/octet-stream")
        .set_payload(std::move(contents));
  });
}
