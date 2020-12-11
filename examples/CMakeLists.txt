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
find_package(fmt REQUIRED)

add_library(
    functions_framework_examples # cmake-format: sort
    hello_cloud_event/hello_cloud_event.cc
    hello_from_namespace/hello_from_namespace.cc
    hello_from_nested_namespace/hello_from_nested_namespace.cc
    hello_gcs/hello_gcs.cc
    hello_multiple_sources/greeting.cc
    hello_multiple_sources/greeting.h
    hello_multiple_sources/hello_multiple_sources.cc
    hello_with_third_party/hello_with_third_party.cc
    hello_world/hello_world.cc
    site/hello_world_content/hello_world_content.cc
    site/hello_world_get/hello_world_get.cc
    site/hello_world_http/hello_world_http.cc
    site/http_cors/http_cors.cc
    site/http_cors_auth/http_cors_auth.cc
    site/http_method/http_method.cc
    site/http_xml/http_xml.cc)
target_link_libraries(functions_framework_examples storage_client fmt::fmt
                      googleapis-c++::functions_framework)

if (BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)
    set(googleapis_functions_framework_examples_unit_tests
        # cmake-format: sort
        cloud_event_examples_test.cc http_examples_test.cc site_test.cc)

    foreach (fname ${googleapis_functions_framework_examples_unit_tests})
        string(REPLACE "/" "_" target "${fname}")
        string(REPLACE ".cc" "" target "${target}")
        add_executable("${target}" ${fname})
        target_link_libraries(
            ${target}
            PRIVATE functions_framework_examples googleapis_functions_framework
                    GTest::gmock_main GTest::gmock GTest::gtest)
        add_test(NAME ${target} COMMAND ${target})
    endforeach ()
endif ()
