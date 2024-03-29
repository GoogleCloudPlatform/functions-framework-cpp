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

// [START functions_tips_lazy_globals]
#include <google/cloud/functions/function.h>
#include <mutex>
#include <string>

namespace gcf = ::google::cloud::functions;

namespace {
// Placeholders to illustrate lazy global initialization.
std::once_flag h_init_flag;
std::string h;
void h_init() { h = "heavy computation"; }
}  // namespace

gcf::Function tips_lazy_globals() {
  return gcf::MakeFunction([](gcf::HttpRequest const& /*request*/) {
    std::call_once(h_init_flag, h_init);
    return gcf::HttpResponse{}.set_payload("Global: " + h);
  });
}
// [END functions_tips_lazy_globals]
