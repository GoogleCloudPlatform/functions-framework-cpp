#!/bin/bash
#
# Copyright 2023 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Make our include guard clean against set -o nounset.
test -n "${CI_CLOUDBUILD_BUILDS_LIB_CMAKE_SH__:-}" || declare -i CI_CLOUDBUILD_BUILDS_LIB_CMAKE_SH__=0
if ((CI_CLOUDBUILD_BUILDS_LIB_CMAKE_SH__++ != 0)); then
  return 0
fi # include guard

function cmake::common_args() {
  local binary="cmake-out"
  if [[ $# -ge 1 ]]; then
    binary="$1"
  fi
  local args
  args=(
    -G Ninja
    -S .
    -B "${binary}"
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  )
  printf "%s\n" "${args[@]}"
}

function ctest::common_args() {
  local binary="cmake-out"
  if [[ $# -ge 1 ]]; then
    binary="$1"
  fi
  local args
  args=(
    --test-dir "${binary}"
    --output-on-failure
    --timeout 60s
  )
  printf "%s\n" "${args[@]}"
}
