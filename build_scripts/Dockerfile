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

FROM gcr.io/gcp-runtimes/ubuntu_18_0_4 AS parent

ARG cnb_uid=1000
ARG cnb_gid=1000
ARG stack_id="google"

# Required by python/runtime.
RUN apt-get update && apt-get install -y --no-install-recommends \
  libexpat1 \
  libffi6 \
  libmpdec2 \
  && apt-get clean && rm -rf /var/lib/apt/lists/*

# Required by dotnet/runtime.
RUN apt-get update && apt-get install -y --no-install-recommends \
  libicu60 \
  && apt-get clean && rm -rf /var/lib/apt/lists/*

# Required by cpp/runtime.
RUN apt-get update \
    && apt-get install -y libc++1-9 \
    && apt-get clean && rm -rf /var/lib/apt/lists/*
LABEL io.buildpacks.stack.id=${stack_id}
RUN groupadd cnb --gid ${cnb_gid} && \
  useradd --uid ${cnb_uid} --gid ${cnb_gid} -m -s /bin/bash cnb

ENV CNB_USER_ID=${cnb_uid}
ENV CNB_GROUP_ID=${cnb_gid}
ENV CNB_STACK_ID=${stack_id}

FROM parent AS gcf-cpp-runtime
ENV PORT 8080
USER cnb

FROM parent AS gcf-cpp-incremental-0
RUN apt-get update \
    && apt install -y build-essential g++-8 gcc-8 git libstdc++-8-dev pkg-config ninja-build tar unzip zip \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 100 \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /layers/cpp
RUN chown cnb /layers/cpp
USER cnb

RUN mkdir -p /layers/cpp/cmake /layers/cpp/vcpkg /layers/cpp/vcpkg-cache /layers/cpp/functions-framework-cpp

RUN cd /layers/cpp/cmake && \
    curl -sSL https://github.com/Kitware/CMake/releases/download/v3.18.4/cmake-3.18.4-Linux-x86_64.tar.gz | \
    tar -xzf - --strip-components=1

RUN cd /layers/cpp/vcpkg && \
    curl -sSL https://github.com/Microsoft/vcpkg/archive/5dc53211caedebf4387d590155ed53ee44161f10.tar.gz | \
    tar -xzf - --strip-components=1 && \
    ./bootstrap-vcpkg.sh -disableMetrics

ENV VCPKG_DEFAULT_BINARY_CACHE=/layers/cpp/vcpkg-cache

# These are needed by the Functions Framework, do them one at a time, easier to
# rebuild the Docker image if one of them fails to download or something.
FROM gcf-cpp-incremental-0 AS gcf-cpp-incremental-1
RUN /layers/cpp/vcpkg/vcpkg install --clean-after-build boost-core
FROM gcf-cpp-incremental-1 AS gcf-cpp-incremental-2
RUN /layers/cpp/vcpkg/vcpkg install --clean-after-build openssl
FROM gcf-cpp-incremental-2 AS gcf-cpp-incremental-3
RUN /layers/cpp/vcpkg/vcpkg install --clean-after-build boost-program-options
FROM gcf-cpp-incremental-3 AS gcf-cpp-incremental-4
RUN /layers/cpp/vcpkg/vcpkg install --clean-after-build boost-asio
FROM gcf-cpp-incremental-4 AS gcf-cpp-incremental-5
RUN /layers/cpp/vcpkg/vcpkg install --clean-after-build boost-beast

# The following are not needed by the Functions Framework, but are used often
# enough that it is a good idea to make them part of the base development
# environment. Note that this automatically pulls abseil, grpc, protobuf, curl,
# and a few other libraries.
FROM gcf-cpp-incremental-5 AS gcf-cpp-incremental-6
RUN /layers/cpp/vcpkg/vcpkg install --clean-after-build curl
FROM gcf-cpp-incremental-6 AS gcf-cpp-incremental-7
RUN /layers/cpp/vcpkg/vcpkg install --clean-after-build grpc
FROM gcf-cpp-incremental-7 AS gcf-cpp-incremental-8
RUN /layers/cpp/vcpkg/vcpkg install --clean-after-build google-cloud-cpp
RUN echo "DISABLED: /layers/cpp/vcpkg/vcpkg install boost"

FROM gcf-cpp-incremental-8 AS gcf-cpp-develop

# TODO(#41) - switch to the actual vcpkg package once created.
RUN mkdir -p /layers/cpp/gcf/cmake /layers/cpp/gcf/vcpkg-overlays
COPY --chown=cnb vcpkg-overlays /layers/cpp/gcf/vcpkg-overlays
RUN /layers/cpp/vcpkg/vcpkg install --clean-after-build \
    --overlay-ports=/layers/cpp/gcf/vcpkg-overlays functions-framework-cpp
