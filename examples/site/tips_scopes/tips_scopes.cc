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

// [START functions_tips_scopes]
#include <google/cloud/functions/http_request.h>
#include <google/cloud/functions/http_response.h>
#include <string>

namespace gcf = ::google::cloud::functions;

namespace {
// Placeholders to illustrate global vs. local initialization
std::string heavy_computation();
std::string light_computation();

std::string h = heavy_computation();
}  // namespace

gcf::HttpResponse tips_scopes(gcf::HttpRequest /*request*/) {  // NOLINT
  auto l = light_computation();
  gcf::HttpResponse response;
  response.set_payload("Global: " + h + ", Local: " + l);
  return response;
}
// [END functions_tips_scopes]

namespace {
std::string heavy_computation() { return "slow"; }

std::string light_computation() { return "fast"; }
}  // namespace
