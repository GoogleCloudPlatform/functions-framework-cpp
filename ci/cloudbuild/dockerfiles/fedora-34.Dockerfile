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

FROM fedora:34
ARG NCPU=4

# Installs the development tools needed by functions-framework-cpp and its
# dependencies.
RUN dnf makecache && \
    dnf install -y abi-compliance-checker autoconf automake \
        ccache clang clang-analyzer clang-tools-extra \
        cmake diffutils doxygen findutils gcc-c++ git \
        lcov libcxx-devel libcxxabi-devel \
        libasan libubsan libtsan libcurl-devel make ninja-build \
        openssl-devel patch python python3.8 \
        python-pip tar unzip w3m wget which zip zlib-devel

# Installs Universal Ctags (which is different than the default "Exuberant
# Ctags"), which is needed by the ABI checker. See https://ctags.io/
WORKDIR /var/tmp/build
RUN curl -sSL https://github.com/universal-ctags/ctags/archive/refs/tags/p5.9.20210418.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    ./autogen.sh && \
    ./configure --prefix=/usr/local && \
    make && \
    make install && \
    cd /var/tmp && rm -fr build

# Installs the abi-dumper with the integer overflow fix from
# https://github.com/lvc/abi-dumper/pull/29. We can switch back to `dnf install
# abi-dumper` once it has the fix.
WORKDIR /var/tmp/build
RUN curl -sSL https://github.com/lvc/abi-dumper/archive/814effec0f20a9613441dfa033aa0a0bc2a96a87.tar.gz | \
    tar -xzf - --strip-components=1 && \
    mv abi-dumper.pl /usr/local/bin/abi-dumper && \
    chmod +x /usr/local/bin/abi-dumper

WORKDIR /var/tmp/gcloud
ARG GOOGLE_CLOUD_CPP_CLOUD_SDK_VERSION="348.0.0"
ARG GOOGLE_CLOUD_CPP_SDK_SHA256="8341a9b21088fd382522be247c7e51c61d8ea4ff86e6ededfa601afd5223e153"
ENV TARBALL="google-cloud-sdk-${GOOGLE_CLOUD_CPP_CLOUD_SDK_VERSION}-linux-x86_64.tar.gz"
RUN curl -sSL "https://dl.google.com/dl/cloudsdk/channels/rapid/downloads/${TARBALL}" -o "${TARBALL}"
RUN echo "${GOOGLE_CLOUD_CPP_SDK_SHA256} ${TARBALL}" | sha256sum --check -
RUN tar x -C /usr/local -f "${TARBALL}"
