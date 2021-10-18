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

#include <google/cloud/functions/http_request.h>
#include <google/cloud/functions/http_response.h>
#include <google/cloud/storage/client.h>
#include <sstream>

using ::google::cloud::functions::HttpRequest;
using ::google::cloud::functions::HttpResponse;
namespace gcs = ::google::cloud::storage;

// Though not used in this example, the request is passed by value to support
// applications that move-out its data.
HttpResponse HelloGcs(HttpRequest request) {  // NOLINT
  auto error = [] {
    return HttpResponse{}.set_result(HttpResponse::kBadRequest);
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

  return HttpResponse{}
      .set_header("Content-Type", "application/octet-stream")
      .set_payload(std::move(contents));
}
