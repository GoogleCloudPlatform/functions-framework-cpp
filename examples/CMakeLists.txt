# ~~~
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
# ~~~

find_package(storage_client REQUIRED)

add_library(
    functions_framework_examples # cmake-format: sort
    hello_from_namespace/hello_from_namespace.cc
    hello_from_nested_namespace/hello_from_nested_namespace.cc
    hello_gcs/hello_gcs.cc
    hello_multiple_sources/greeting.cc
    hello_multiple_sources/greeting.h
    hello_multiple_sources/hello_multiple_sources.cc
    hello_with_third_party/hello_with_third_party.cc
    hello_world/hello_world.cc)
target_link_libraries(functions_framework_examples storage_client
                      googleapis-c++::functions_framework)
