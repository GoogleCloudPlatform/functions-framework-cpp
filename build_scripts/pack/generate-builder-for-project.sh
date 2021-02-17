#!/usr/bin/env bash
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

set -eu

if [[ "$#" -ne 2 ]]; then
  echo 2> "Usage: $(basename "$0") <project-id> <destination-directory>"
  exit 1
fi

readonly PROJECT_ID="${1}"
readonly DEST="${2}"

mkdir -p "${DEST}/buildpack/bin"

cat >"${DEST}/builder.toml" <<_EOF_
[[buildpacks]]
uri = "buildpack"

[[order]]
    [[order.group]]
    id = "com.google.buildpack.cpp.project"
    version = "0.4.0"

# Stack that will be used by the builder
[stack]
id = "google"
run-image   = "gcr.io/${PROJECT_ID}/functions-framework-cpp/run-image:latest"
build-image = "gcr.io/${PROJECT_ID}/functions-framework-cpp/build-image:latest"
_EOF_

cat >"${DEST}/buildpack/buildpack.toml" <<_EOF_
api = "0.2"

[buildpack]
id = "com.google.buildpack.cpp.project"
version = "0.4.0"
name = "Functions Framework C++ Buildpack for GCP Project ${PROJECT_ID}"

[[stacks]]
id = "google"
_EOF_

cat >"${DEST}/buildpack/bin/detect" <<'_EOF_'
#!/bin/bash

set -eu

if ! (compgen -G "*.cc" >/dev/null ||
  compgen -G "*.cpp" >/dev/null ||
  compgen -G "*.cxx" >/dev/null ||
  [[ ! -f CMakeLists.txt ]]); then
  exit 100
fi

exit 0
_EOF_

cat >"${DEST}/buildpack/bin/build" <<'_SCRIPT_EOF_'
#!/bin/bash

set -eu

set -eu

echo "---> Functions Framework C++ Buildpack"

layers="$1"

echo "-----> Setup vcpkg"
export VCPKG_DEFAULT_BINARY_CACHE="${layers}/vcpkg-cache"
export VCPKG_DEFAULT_TRIPLET=x64-linux-nodebug
export VCPKG_ROOT="${layers}/vcpkg"

if [[ ! -d "${VCPKG_ROOT}" ]]; then
  echo "-----> Install vcpkg"
  mkdir -p "${VCPKG_ROOT}"
  curl -sSL https://github.com/Microsoft/vcpkg/archive/65e5ea1df685a5362e70367bef4dbf827addff31.tar.gz | \
    tar -C "${VCPKG_ROOT}" -xzf - --strip-components=1
  cat >"${VCPKG_ROOT}/triplets/x64-linux-nodebug.cmake" <<_EOF_
set(VCPKG_BUILD_TYPE release)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_TARGET_ARCHITECTURE x64)
_EOF_
  cp -r /usr/local/bin/vcpkg "${VCPKG_ROOT}"
cat >"${layers}/vcpkg.toml" <<EOL
build = true
cache = true
launch = false
EOL
fi

if [[ ! -d "${layers}/vcpkg-cache" ]]; then
  echo "-----> Restore cache from build image"
  cp -r /var/cache/vcpkg-cache "${layers}/vcpkg-cache"
cat >"${layers}/vcpkg-cache.toml" <<EOL
build = true
cache = true
launch = false
EOL
fi

echo "-----> Setup build directory"
cat >"${layers}/source.toml" <<EOL
build = true
cache = false
launch = false
EOL

cp -r /usr/local/share/gcf/build_scripts/cmake "${layers}/source"
if [[ -r vcpkg.json ]]; then
  cp vcpkg.json "${layers}/source"
else
  cat >"${layers}/source/vcpkg.json" <<_EOF_
{
  "name": "auto-generated-vcpkg-json",
  "version-string": "unversioned",
  "dependencies": [ "functions-framework-cpp" ]
}
_EOF_
fi
/usr/local/share/gcf/build_scripts/generate-wrapper.sh \
    "${TARGET_FUNCTION}" "${FUNCTION_SIGNATURE_TYPE:-http}" >"${layers}/source/main.cc"

echo "-----> Configure Function"
cat >"${layers}/binary.toml" <<EOL
build = true
cache = true
launch = false
EOL

/usr/local/bin/cmake -S "${layers}/source" -B "${layers}/binary" -GNinja -DCMAKE_MAKE_PROGRAM=/usr/local/bin/ninja \
  -DCNB_APP_DIR="${PWD}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="${layers}/local" \
  -DVCPKG_TARGET_TRIPLET="${VCPKG_DEFAULT_TRIPLET}" \
  -DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
/usr/local/bin/cmake --build "${layers}/binary" --target install

cat >"${layers}/local.toml" <<EOL
launch = true
cache = false
build = false
EOL

cat >"${layers}/launch.toml" <<EOL
[[processes]]
type = "web"
command = "${layers}/local/bin/function"
EOL
_SCRIPT_EOF_
