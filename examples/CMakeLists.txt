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
find_package(pubsub_client REQUIRED)
find_package(fmt REQUIRED)
find_package(Boost REQUIRED COMPONENTS log)

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
    site/concepts_after_response/concepts_after_response.cc
    site/concepts_after_timeout/concepts_after_timeout.cc
    site/concepts_filesystem/concepts_filesystem.cc
    site/concepts_request/concepts_request.cc
    site/concepts_stateless/concepts_stateless.cc
    site/env_vars/env_vars.cc
    site/hello_world_content/hello_world_content.cc
    site/hello_world_get/hello_world_get.cc
    site/hello_world_http/hello_world_http.cc
    site/hello_world_pubsub/hello_world_pubsub.cc
    site/hello_world_storage/hello_world_storage.cc
    site/http_cors/http_cors.cc
    site/http_cors_auth/http_cors_auth.cc
    site/http_method/http_method.cc
    site/http_xml/http_xml.cc
    site/tips_gcp_apis/tips_gcp_apis.cc
    site/tips_infinite_retries/tips_infinite_retries.cc
    site/tips_lazy_globals/tips_lazy_globals.cc
    site/tips_retry/tips_retry.cc
    site/tips_scopes/tips_scopes.cc)
target_link_libraries(
    functions_framework_examples storage_client fmt::fmt
    googleapis-c++::functions_framework googleapis-c++::pubsub_client)

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
            PRIVATE functions_framework_examples
                    googleapis_functions_framework
                    Boost::filesystem
                    Boost::log
                    GTest::gmock_main
                    GTest::gmock
                    GTest::gtest)
        add_test(NAME ${target} COMMAND ${target})
    endforeach ()
endif ()

add_subdirectory(site/testing_http)
add_subdirectory(site/testing_pubsub)
