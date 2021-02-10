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

FROM parent AS gcf-cpp-incremental
RUN apt-get update \
    && apt install -y  --no-install-recommends \
       build-essential g++-8 gcc-8 git libstdc++-8-dev pkg-config tar unzip zip \
    && apt-get clean && rm -rf /var/lib/apt/lists/*
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 100

WORKDIR /usr/local
RUN curl -sSL https://github.com/Kitware/CMake/releases/download/v3.19.4/cmake-3.19.4-Linux-x86_64.tar.gz | \
    tar -xzf - --strip-components=1

RUN curl -sSL https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-linux.zip | \
    funzip >/usr/local/bin/ninja && \
    chmod 755 /usr/local/bin/ninja
WORKDIR /usr/local/vcpkg
RUN curl -sSL https://github.com/Microsoft/vcpkg/archive/65e5ea1df685a5362e70367bef4dbf827addff31.tar.gz | \
    tar -xzf - --strip-components=1 && \
    ./bootstrap-vcpkg.sh -disableMetrics -useSystemBinaries && \
    rm -fr toolsrc/build.rel downloads/*

WORKDIR /var/cache/vcpkg-cache
ENV VCPKG_DEFAULT_BINARY_CACHE=/var/cache/vcpkg-cache

# These are needed by the Functions Framework, do them one at a time, easier to
# rebuild the Docker image if one of them fails to download or something.
RUN /usr/local/vcpkg/vcpkg install --clean-after-build abseil
RUN /usr/local/vcpkg/vcpkg install --clean-after-build boost-core
RUN /usr/local/vcpkg/vcpkg install --clean-after-build openssl
RUN /usr/local/vcpkg/vcpkg install --clean-after-build boost-program-options
RUN /usr/local/vcpkg/vcpkg install --clean-after-build boost-asio
RUN /usr/local/vcpkg/vcpkg install --clean-after-build boost-beast
RUN /usr/local/vcpkg/vcpkg install --clean-after-build boost-serialization
RUN /usr/local/vcpkg/vcpkg install --clean-after-build nlohmann-json

# The following are not needed by the Functions Framework, but are used often
# enough that it is a good idea to make them part of the base development
# environment. Note that this automatically pulls abseil, grpc, protobuf, curl,
# and a few other libraries.
RUN /usr/local/vcpkg/vcpkg install --clean-after-build curl
RUN /usr/local/vcpkg/vcpkg install --clean-after-build protobuf
RUN /usr/local/vcpkg/vcpkg install --clean-after-build grpc
RUN /usr/local/vcpkg/vcpkg install --clean-after-build google-cloud-cpp

# TODO(#41) - switch to the actual vcpkg package once created.
COPY . /usr/local/share/gcf
RUN find /usr/local/share/gcf -type f | xargs chmod 644
RUN find /usr/local/share/gcf -type d | xargs chmod 755
RUN chmod 755 /usr/local/share/gcf/build_scripts/generate-wrapper.sh

RUN VCPKG_OVERLAY_PORTS=/usr/local/share/gcf/build_scripts/vcpkg-overlays /usr/local/vcpkg/vcpkg install functions-framework-cpp
RUN /usr/local/vcpkg/vcpkg list | awk '{print $1}' | xargs /usr/local/vcpkg/vcpkg remove --recurse

FROM parent AS gcf-cpp-develop
RUN apt-get update \
    && apt install -y  --no-install-recommends \
       build-essential g++-8 gcc-8 git libstdc++-8-dev pkg-config tar unzip zip \
    && apt-get clean && rm -rf /var/lib/apt/lists/*
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 100

COPY --from=gcf-cpp-incremental /usr/local/bin/cmake   /usr/local/bin/cmake
COPY --from=gcf-cpp-incremental /usr/local/bin/ctest   /usr/local/bin/ctest
COPY --from=gcf-cpp-incremental /usr/local/bin/ninja   /usr/local/bin/ninja
COPY --from=gcf-cpp-incremental /usr/local/vcpkg/vcpkg /usr/local/bin/vcpkg
COPY --from=gcf-cpp-incremental /usr/local/share /usr/local/share
COPY --from=gcf-cpp-incremental /var/cache/vcpkg-cache /var/cache/vcpkg-cache

USER cnb
