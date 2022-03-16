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

set -euo pipefail

declare -A files=(
  ["README.md"]="examples/hello_world/hello_world.cc"
  ["examples/site/howto_create_container/README.md"]="examples/site/hello_world_http/hello_world_http.cc"
  ["examples/site/howto_deploy_cloud_event/README.md"]="examples/site/hello_world_pubsub/hello_world_pubsub.cc"
  ["examples/site/howto_deploy_to_cloud_run/README.md"]="examples/site/hello_world_http/hello_world_http.cc"
  ["examples/site/testing_pubsub/README.md"]="examples/site/hello_world_pubsub/hello_world_pubsub.cc"
  ["examples/site/testing_storage/README.md"]="examples/site/hello_world_storage/hello_world_storage.cc"
)

for markdown in "${!files[@]}"; do
  source="${files[$markdown]}"
  (
    sed '/<!-- inject-snippet-start -->/q' "${markdown}"
    echo "[snippet source]: /${source}"
    echo '```cc'
    # Dumps the contents of the source file starting at the first #include, so we
    # skip the license header comment.
    sed -n -e '/END .*quickstart/,$d' -e '/^#/,$p' "${source}" | sed -e '/^\/\//d' -e 's;  // NOLINT;;'
    echo '```'
    sed -n '/<!-- inject-snippet-end -->/,$p' "${markdown}"
  ) | sponge "${markdown}"
done
