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

if (BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)
    find_package(fmt CONFIG REQUIRED)
    find_package(google_cloud_cpp_pubsub REQUIRED)

    set(functions_framework_cpp_examples_unit_tests
        # cmake-format: sort
        pubsub_integration_test.cc pubsub_unit_test.cc)

    foreach (fname ${functions_framework_cpp_examples_unit_tests})
        string(REPLACE "/" "_" target "${fname}")
        string(REPLACE ".cc" "" target "${target}")
        add_executable("${target}" ${fname})
        target_link_libraries(
            ${target}
            PRIVATE functions_framework_examples
                    functions-framework-cpp::framework
                    Boost::filesystem
                    Boost::log
                    GTest::gmock_main
                    GTest::gmock
                    GTest::gtest)
        add_test(NAME ${target} COMMAND ${target})
    endforeach ()

    add_executable(pubsub_integration_server pubsub_integration_server.cc)
    target_link_libraries(
        pubsub_integration_server
        PRIVATE functions_framework_examples functions-framework-cpp::framework
                Boost::filesystem Boost::log)

    add_executable(pubsub_system_test pubsub_system_test.cc)
    target_link_libraries(
        pubsub_system_test
        PRIVATE google-cloud-cpp::pubsub fmt::fmt Boost::log
                GTest::gmock_main GTest::gmock GTest::gtest)
endif ()
