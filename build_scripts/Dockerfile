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

# Install the system tools needed to compile C++. This needs to go into the
# `build-image` of the buildpack.
FROM parent AS gcf-cpp-incremental-0
RUN apt-get update \
    && apt install -y  --no-install-recommends \
       build-essential g++-8 gcc-8 git libstdc++-8-dev pkg-config python3 tar unzip zip \
    && apt-get clean && rm -rf /var/lib/apt/lists/*
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 100

# Install cmake, ninja and vcpkg. The first two are build systems for C++, the
# latter is the package manager we use. In an open source builder these would
# get downloaded dynamically, for builders created by the user they can be
# part of the builder to speed up the first build of each function.
FROM gcf-cpp-incremental-0 AS gcf-cpp-incremental-1

WORKDIR /usr/local
RUN curl -sSL https://github.com/Kitware/CMake/releases/download/v3.19.4/cmake-3.19.4-Linux-x86_64.tar.gz | \
    tar -xzf - --strip-components=1

RUN curl -sSL https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-linux.zip | \
    funzip >/usr/local/bin/ninja && \
    chmod 755 /usr/local/bin/ninja

WORKDIR /usr/local/vcpkg
RUN curl -sSL https://github.com/Microsoft/vcpkg/archive/2024.09.30.tar.gz | \
    tar -xzf - --strip-components=1 && \
    ./bootstrap-vcpkg.sh -disableMetrics -useSystemBinaries && \
    rm -fr toolsrc/build.rel downloads/*
# TODO(#249) - for now duplicate this code from /pack/bin/build, this should go away once we
#   split the "production" vs. "development" builders.
RUN { \
    echo 'set(VCPKG_BUILD_TYPE release)'; \
    echo 'set(VCPKG_CMAKE_SYSTEM_NAME Linux)'; \
    echo 'set(VCPKG_CRT_LINKAGE dynamic)'; \
    echo 'set(VCPKG_LIBRARY_LINKAGE static)'; \
    echo 'set(VCPKG_TARGET_ARCHITECTURE x64)'; \
} >triplets/x64-linux-nodebug.cmake

# Warm up the vcpkg cache with all the dependencies for `functions-framework-cpp` and some
# commonly use packages, like `google-cloud-cpp`.
WORKDIR /var/cache/vcpkg-cache
ENV VCPKG_DEFAULT_BINARY_CACHE=/var/cache/vcpkg-cache

FROM gcf-cpp-incremental-1 AS gcf-cpp-incremental-2

# These are needed by the Functions Framework, do them one at a time, easier to
# rebuild the Docker image if one of them fails to download or something.
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug abseil
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug openssl
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug nlohmann-json

# The build spends a lot of time downloading boost deps multiple times unless we
# fetch them before hand.
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-vcpkg-helpers

RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-algorithm
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-align
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-any
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-array
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-asio
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-assert
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-atomic
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-beast
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-bind
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-build
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-chrono
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-concept-check
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-config
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-container
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-container-hash
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-context
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-conversion
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-core
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-coroutine
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-date-time
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-describe
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-detail
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-endian
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-exception
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-function
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-function-types
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-functional
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-fusion
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-integer
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-intrusive
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-io
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-iterator
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-lexical-cast
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-logic
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-modular-build-helper
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-move
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-mp11
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-mpl
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-numeric-conversion
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-optional
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-phoenix
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-pool
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-predef
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-preprocessor
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-program-options
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-proto
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-range
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-ratio
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-rational
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-regex
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-serialization
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-smart-ptr
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-spirit
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-static-assert
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-static-string
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-system
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-thread
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-throw-exception
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-tokenizer
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-tuple
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-type-index
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-type-traits
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-typeof
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-uninstall
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-unordered
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-utility
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-variant
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug boost-variant2

# The following are not needed by the Functions Framework, but are used often
# enough that it is a good idea to make them part of the base development
# environment. Note that this automatically pulls abseil, grpc, protobuf, curl,
# and a few other libraries.
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug curl
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug protobuf
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug grpc
RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug google-cloud-cpp

# Warmup the vcpkg cache for `functions-framework-cpp` using the release version
# of the framework, this is the recommended path for users of the framework.
FROM gcf-cpp-incremental-2 AS gcf-cpp-incremental-3

RUN /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug functions-framework-cpp

# This is the development image we recommend users create in their workstation.
# It includes all the development tools, including cmake, ninja and vcpkg, as
# well as binary caches for the latest release of `functions-framework-cpp` and
# its dependencies, as well as for `google-cloud-cpp`.
FROM gcf-cpp-incremental-1 AS gcf-cpp-develop

COPY --from=gcf-cpp-incremental-3 /usr/local/bin/cmake   /usr/local/bin/cmake
COPY --from=gcf-cpp-incremental-3 /usr/local/bin/ctest   /usr/local/bin/ctest
COPY --from=gcf-cpp-incremental-3 /usr/local/bin/ninja   /usr/local/bin/ninja
COPY --from=gcf-cpp-incremental-3 /usr/local/vcpkg/vcpkg /usr/local/bin/vcpkg
COPY --from=gcf-cpp-incremental-3 /var/cache/vcpkg-cache /var/cache/vcpkg-cache

USER cnb

# Warmup the vcpkg cache for `functions-framework-cpp` using the *current*
# version of the framework, this is used in the CI build.
FROM gcf-cpp-incremental-2 AS gcf-cpp-ci-0

COPY . /usr/local/share/gcf
RUN find /usr/local/share/gcf -type f | xargs chmod 644
RUN find /usr/local/share/gcf -type d | xargs chmod 755
RUN VCPKG_OVERLAY_PORTS=/usr/local/share/gcf/build_scripts/vcpkg-overlays \
    /usr/local/vcpkg/vcpkg install --feature-flags=-manifests --triplet x64-linux-nodebug functions-framework-cpp

# This is the image used for the CI builds, includes a binary cache of the
# *current* version of `functions-framework-cpp`, as well as binary caches of
# its dependencies and other packages used in the examples.
FROM gcf-cpp-incremental-1 AS gcf-cpp-ci

# Copy the framework to /usr/local/share/gcf, that is just a lazy way to
# get all the scripts we need for both the CI and regular builds.

COPY --from=gcf-cpp-ci-0 /usr/local/bin/cmake   /usr/local/bin/cmake
COPY --from=gcf-cpp-ci-0 /usr/local/bin/ctest   /usr/local/bin/ctest
COPY --from=gcf-cpp-ci-0 /usr/local/bin/ninja   /usr/local/bin/ninja
COPY --from=gcf-cpp-ci-0 /usr/local/vcpkg/vcpkg /usr/local/bin/vcpkg
COPY --from=gcf-cpp-ci-0 /usr/local/share /usr/local/share
COPY --from=gcf-cpp-ci-0 /var/cache/vcpkg-cache /var/cache/vcpkg-cache

USER cnb

FROM gcf-cpp-incremental-0 AS gcf-cpp-minimal
USER cnb
