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
ARG TARGET_SIGNATURE
COPY --chown=cnb --from=parent /layers/cpp/gcf/cmake /workspace
COPY --chown=cnb . /workspace/application

RUN /layers/cpp/gcf/generate-wrapper.sh "${TARGET_FUNCTION}" "${TARGET_SIGNATURE}" >/workspace/main.cc

RUN /layers/cpp/cmake/bin/cmake -S /workspace -B /var/tmp/build -GNinja \
    -DCMAKE_INSTALL_PREFIX=/var/tmp/install \
    -DCMAKE_TOOLCHAIN_FILE=/layers/cpp/vcpkg/scripts/buildsystems/vcpkg.cmake
RUN /layers/cpp/cmake/bin/cmake --build /var/tmp/build --target install

FROM gcf-cpp-runtime AS runtime
COPY --from=application /var/tmp/install/bin/application /r/application

ENTRYPOINT /r/application
