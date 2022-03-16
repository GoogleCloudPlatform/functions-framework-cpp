#!/bin/bash
#
# Copyright 2021 Google LLC
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

set -euo pipefail

export CC=gcc
export CXX=g++

source "$(dirname "$0")/../../lib/init.sh"
source module ci/lib/io.sh
source module ci/cloudbuild/builds/lib/vcpkg.sh

INSTALL_PREFIX=/var/tmp/functions-framework-cpp
vcpkg_root="$(vcpkg::root_dir)"
# abi-dumper wants us to use -Og, but that causes bogus warnings about
# uninitialized values with GCC, so we disable that warning with
# -Wno-maybe-uninitialized. See also:
#     https://github.com/googleapis/google-cloud-cpp/issues/6313
io::log_h2 "Configuring, building, and installing the C++ Functions Framework"
cmake -GNinja \
  -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
  -DCMAKE_INSTALL_MESSAGE=NEVER \
  -DBUILD_SHARED_LIBS=ON \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-Og -Wno-maybe-uninitialized" \
  -DCMAKE_TOOLCHAIN_FILE="${vcpkg_root}/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_FEATURE_FLAGS="versions,manifest" \
  -S . -B cmake-out
cmake --build cmake-out
cmake --install cmake-out

# Uses `abi-dumper` to dump the ABI for the given library, which should be
# installed at the given prefix. This function will be called from a subshell,
# so it cannot use other variables or functions (including io::log*).
function dump_abi() {
  local library="$1"
  local prefix="$2"
  io::log_h2 "Dumping ${library} (may be slow)..."
  version="$(env PKG_CONFIG_PATH="${prefix}/lib64/pkgconfig" pkg-config --modversion "${library}")"
  abi-dumper "${prefix}/lib64/lib${library}.so.${version}" \
    -public-headers "${prefix}/include" \
    -lver "actual" \
    -o "cmake-out/${library}.actual.abi.dump"
}

dump_abi "functions_framework_cpp" "${INSTALL_PREFIX}"

io::log_h2 "Searching for API changes in functions_framework_cpp"
actual_dump_file="functions_framework_cpp.actual.abi.dump"
expected_dump_file="functions_framework_cpp.expected.abi.dump"
expected_dump_path="${PROJECT_ROOT}/ci/abi-dumps/${expected_dump_file}.gz"
if [[ -r "${expected_dump_path}" ]]; then
  zcat "${expected_dump_path}" >"cmake-out/${expected_dump_file}"
  report="cmake-out/compat_reports/functions_framework_cpp/expected_to_actual/src_compat_report.html"
  # We ignore all symbols in internal namespaces, because these are not part
  # of our public API. We do this by specifying a regex that matches against
  # the mangled symbol names. For example, 8 is the number of characters in
  # the string "internal", and it should again be followed by some other
  # number indicating the length of the symbol within the "internal"
  # namespace. See: https://en.wikipedia.org/wiki/Name_mangling
  if ! abi-compliance-checker \
    -skip-internal-symbols "(18functions_internal)\d" \
    -report-path "${report}" \
    -src -l "functions_framework_cpp" \
    -old "cmake-out/${expected_dump_file}" \
    -new "cmake-out/${actual_dump_file}"; then
    io::log_red "ABI Compliance error: functions_framework_cpp"
    io::log "Report file: ${report}"
    w3m -dump "${report}"
    exit 1
  fi
fi

# Replaces the (old) expected dump file with the (new) actual one.
gzip -n "cmake-out/${actual_dump_file}"
mv -f "cmake-out/${actual_dump_file}.gz" "${expected_dump_path}"
