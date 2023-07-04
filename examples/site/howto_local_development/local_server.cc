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

#include <google/cloud/functions/framework.h>
#include <cstdlib>

namespace gcf = ::google::cloud::functions;

namespace {

gcf::Function HelloLocal() {
  return gcf::MakeFunction([](gcf::HttpRequest const& /*request*/) {
    return gcf::HttpResponse{}
        .set_header("Content-Type", "text/plain")
        .set_payload("Hello World\n");
  });
}

}  // namespace

int main(int argc, char* argv[]) { return gcf::Run(argc, argv, HelloLocal()); }
