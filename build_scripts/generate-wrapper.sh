#!/bin/bash
# Copyright 2020 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -eu

if [[ $# -ne 2 ]]; then
  >&2 echo "Usage: $(basename "${0}") <TARGET_FUNCTION> <TARGET_SIGNATURE>"
  exit 1
fi

generate_main_no_namespace() {
  cat <<_EOF_
#include <google/cloud/functions/internal/framework.h>

namespace gcf = ::google::cloud::functions;
namespace gcf_internal = ::google::cloud::functions_internal;

extern gcf::HttpResponse ${TARGET_FUNCTION}(gcf::HttpRequest);

int main(int argc, char* argv[]) {
  return gcf_internal::Run(argc, argv, gcf_internal::HttpFunction(${TARGET_FUNCTION});
}
_EOF_
}

generate_main() {
  declare full
  declare dir
  declare namespace
  declare function
  full="$(sed -e 's;::;/;g' <<<"${1}")"
  dir="$(dirname "${full}")"
  namespace="$(sed -e 's;/;::;g' <<<"${dir}")"
  function="$(basename "${full}")"
  if [[ -z "${namespace}" ]]; then
    generate_main_no_namespace
    return
  fi

  cat <<_EOF_
#include <google/cloud/functions/internal/framework.h>

namespace gcf = ::google::cloud::functions;
namespace gcf_internal = ::google::cloud::functions_internal;

namespace ${namespace} {
  extern gcf::HttpResponse ${function}(gcf::HttpRequest);
} // namespace

int main(int argc, char* argv[]) {
  return gcf_internal::Run(argc, argv, gcf_internal::HttpFunction(${1}));
}
_EOF_
}

generate_main "${1}" "${2}"
