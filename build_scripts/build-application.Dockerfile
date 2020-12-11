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

FROM gcf-cpp-develop AS parent

FROM parent AS application
ARG TARGET_FUNCTION
ARG FUNCTION_SIGNATURE_TYPE="http"
COPY --chown=cnb --from=parent /layers/cpp/gcf/cmake /workspace
COPY --chown=cnb . /workspace/application

ENV VCPKG_DEFAULT_BINARY_CACHE=/layers/cpp/vcpkg-cache
ENV VCPKG_OVERLAY_PORTS=/layers/cpp/gcf/vcpkg-overlays

RUN if [ -r /workspace/application/vcpkg.json ]; then cp /workspace/application/vcpkg.json /workspace; fi \
    && /layers/cpp/gcf/generate-wrapper.sh "${TARGET_FUNCTION}" "${FUNCTION_SIGNATURE_TYPE}" >/workspace/main.cc \
    && /layers/cpp/cmake/bin/cmake -S /workspace -B /var/tmp/build -GNinja \
           -DCMAKE_INSTALL_PREFIX=/var/tmp/install \
           -DCMAKE_TOOLCHAIN_FILE=/layers/cpp/vcpkg/scripts/buildsystems/vcpkg.cmake \
    && /layers/cpp/cmake/bin/cmake --build /var/tmp/build --target install

FROM gcf-cpp-runtime AS runtime
COPY --from=application /var/tmp/install/bin/function /r/application

ENTRYPOINT ["/r/application"]
