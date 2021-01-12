#!/usr/bin/env bash
# Copyright 2020 Google LLC
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

cat <<'_EOF_'
# WARNING: DO NOT EDIT THIS FILE
# This file is automatically generated by ./build-examples.sh
timeout: 3600s
options:
  machineType: 'N1_HIGHCPU_32'
  diskSizeGb: 512

steps:
  # Create a docker image for the buildpacks `pack` tool
  - name: 'gcr.io/cloud-builders/git'
    args: [
      'clone', '--depth=1',
      'https://github.com/GoogleCloudPlatform/cloud-builders-community',
      'third_party/cloud-builders-community',
    ]
    waitFor: ['-']
    id: 'clone-cloud-builders-community'
  - name: 'gcr.io/kaniko-project/executor:latest'
    args: [
        "--context=dir:///workspace/third_party/cloud-builders-community/pack/",
        "--dockerfile=Dockerfile",
        "--destination=gcr.io/${PROJECT_ID}/pack:${SHORT_SHA}",
        "--cache=true",
        "--cache-ttl=48h"
    ]
    waitFor: ['clone-cloud-builders-community']

  # Create the docker images for the buildspacks builder.
  - name: 'gcr.io/kaniko-project/executor:latest'
    args: [
        "--context=dir:///workspace/build_scripts/",
        "--dockerfile=Dockerfile",
        "--destination=gcr.io/${PROJECT_ID}/functions-framework-cpp/runtime:${SHORT_SHA}",
        "--target=gcf-cpp-runtime",
        "--cache=true",
        "--cache-ttl=48h"
    ]
    waitFor: ['-']
    timeout: 1800s
  - name: 'gcr.io/cloud-builders/docker'
    args: ['pull', 'gcr.io/${PROJECT_ID}/functions-framework-cpp/runtime:${SHORT_SHA}']
  - name: 'gcr.io/kaniko-project/executor:latest'
    args: [
        "--context=dir:///workspace/build_scripts/",
        "--dockerfile=Dockerfile",
        "--destination=gcr.io/${PROJECT_ID}/functions-framework-cpp/develop:${SHORT_SHA}",
        "--cache=true",
        "--cache-ttl=48h"
    ]
    waitFor: ['-']
    timeout: 1800s
  - name: 'gcr.io/cloud-builders/docker'
    args: ['pull', 'gcr.io/${PROJECT_ID}/functions-framework-cpp/develop:${SHORT_SHA}']
    # Setup local names for the builder images.
  - name: 'gcr.io/cloud-builders/docker'
    args: ['tag', 'gcr.io/${PROJECT_ID}/functions-framework-cpp/develop:${SHORT_SHA}', 'gcf-cpp-develop']
  - name: 'gcr.io/cloud-builders/docker'
    args: ['tag', 'gcr.io/${PROJECT_ID}/functions-framework-cpp/runtime:${SHORT_SHA}', 'gcf-cpp-runtime']

  # Create the buildpacks builder, and make it the default.
  - name: 'gcr.io/${PROJECT_ID}/pack:${SHORT_SHA}'
    args: ['create-builder', 'gcf-cpp-builder:bionic', '--builder-config', 'pack/builder.toml', ]
  - name: 'gcr.io/${PROJECT_ID}/pack:${SHORT_SHA}'
    args: ['trust-builder', 'gcf-cpp-builder:bionic', ]
  - name: 'gcr.io/${PROJECT_ID}/pack:${SHORT_SHA}'
    args: ['set-default-builder', 'gcf-cpp-builder:bionic', ]
    id: 'gcf-builder-ready'

  # Build the examples using the builder. Keep these in alphabetical order.
  - name: 'gcr.io/${PROJECT_ID}/pack:${SHORT_SHA}'
    waitFor: ['gcf-builder-ready']
    args: ['build',
      '--clear-cache',
      '--env', 'FUNCTION_SIGNATURE_TYPE=cloudevent',
      '--env', 'TARGET_FUNCTION=HelloCloudEvent',
      '--path', 'examples/hello_cloud_event',
      'hello-cloud-event',
    ]

  - name: 'gcr.io/${PROJECT_ID}/pack:${SHORT_SHA}'
    waitFor: ['gcf-builder-ready']
    args: ['build',
      '--clear-cache',
      '--env', 'FUNCTION_SIGNATURE_TYPE=http',
      '--env', 'TARGET_FUNCTION=hello_from_namespace::HelloWorld',
      '--path', 'examples/hello_from_namespace',
      'hello-from-namespace',
    ]

  - name: 'gcr.io/${PROJECT_ID}/pack:${SHORT_SHA}'
    waitFor: ['gcf-builder-ready']
    args: ['build',
      '--clear-cache',
      '--env', 'FUNCTION_SIGNATURE_TYPE=http',
      '--env', 'TARGET_FUNCTION=::hello_from_namespace::HelloWorld',
      '--path', 'examples/hello_from_namespace',
      'hello-from-namespace-rooted',
    ]

  - name: 'gcr.io/${PROJECT_ID}/pack:${SHORT_SHA}'
    waitFor: ['gcf-builder-ready']
    args: ['build',
      '--clear-cache',
      '--path', 'examples/hello_from_nested_namespace',
      '--env', 'FUNCTION_SIGNATURE_TYPE=http',
      '--env', 'TARGET_FUNCTION=hello_from_nested_namespace::ns0::ns1::HelloWorld',
      'hello-from-nested-namespace',
    ]

  - name: 'gcr.io/${PROJECT_ID}/pack:${SHORT_SHA}'
    waitFor: ['gcf-builder-ready']
    args: ['build',
      '--clear-cache',
      '--env', 'FUNCTION_SIGNATURE_TYPE=http',
      '--env', 'TARGET_FUNCTION=HelloMultipleSources',
      '--path', 'examples/hello_multiple_sources',
      'hello-multiple-sources',
    ]

  - name: 'gcr.io/${PROJECT_ID}/pack:${SHORT_SHA}'
    waitFor: ['gcf-builder-ready']
    args: ['build',
      '--clear-cache',
      '--env', 'FUNCTION_SIGNATURE_TYPE=http',
      '--env', 'TARGET_FUNCTION=HelloGcs',
      '--path', 'examples/hello_gcs',
      'hello-gcs',
    ]

  - name: 'gcr.io/${PROJECT_ID}/pack:${SHORT_SHA}'
    waitFor: ['gcf-builder-ready']
    args: ['build',
      '--clear-cache',
      '--env', 'FUNCTION_SIGNATURE_TYPE=http',
      '--env', 'TARGET_FUNCTION=HelloWithThirdParty',
      '--path', 'examples/hello_with_third_party',
      'hello-with-third-party',
    ]

  - name: 'gcr.io/${PROJECT_ID}/pack:${SHORT_SHA}'
    waitFor: ['gcf-builder-ready']
    args: ['build',
      '--clear-cache',
      '--env', 'FUNCTION_SIGNATURE_TYPE=http',
      '--env', 'TARGET_FUNCTION=HelloWorld',
      '--path', 'examples/hello_world',
      'hello-world',
    ]

  - name: 'gcr.io/${PROJECT_ID}/pack:${SHORT_SHA}'
    waitFor: ['gcf-builder-ready']
    args: ['build',
      '--clear-cache',
      '--env', 'FUNCTION_SIGNATURE_TYPE=http',
      '--env', 'TARGET_FUNCTION=::HelloWorld',
      '--path', 'examples/hello_world',
      'hello-world-rooted',
    ]

  # Build the cloud site examples, these are more amenable to a for loop by design.
_EOF_

for example in examples/site/*; do
  function="$(basename "${example}")"
  signature="http"
  if grep -q gcf::CloudEvent ${example}/*; then
    signature="cloudevent"
  fi
  cat <<_EOF_
  - name: 'gcr.io/\${PROJECT_ID}/pack:\${SHORT_SHA}'
    waitFor: ['gcf-builder-ready']
    args: ['build',
      '--clear-cache',
      '--env', 'FUNCTION_SIGNATURE_TYPE=${signature}',
      '--env', 'TARGET_FUNCTION=${function}',
      '--path', '${example}',
      'site-${function}',
    ]
_EOF_
done
