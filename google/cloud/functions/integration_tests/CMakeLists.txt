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

find_package(GTest CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem)

set(functions_framework_cpp_integration_tests
    # cmake-format: sort
    basic_integration_test.cc cloud_event_integration_test.cc)

foreach (fname ${functions_framework_cpp_integration_tests})
    string(REPLACE "/" "_" target "${fname}")
    string(REPLACE ".cc" "" target "${target}")
    add_executable("${target}" ${fname})
    target_link_libraries(
        ${target} PRIVATE functions-framework-cpp::framework GTest::gmock
                          GTest::gtest Boost::filesystem)
    functions_framework_cpp_add_common_options(${target})
    # I (coryan) am not sure why this is needed, but it only affects the tests,
    # so I am more willing to tolerate it. I think normally Boost header-only
    # libraries automatically declare when they need additional libraries with
    # MSVC, but it does not seem to work here.
    target_compile_definitions("${target}" PRIVATE BOOST_UUID_FORCE_AUTO_LINK)
    add_test(NAME ${target} COMMAND ${target})
    if (WIN32)
        get_target_property(FRAMEWORK_DIR functions-framework-cpp::framework
                            BINARY_DIR)
        set_tests_properties(
            ${target} PROPERTIES ENVIRONMENT "PATH=${CMAKE_BINARY_DIR};${FRAMEWORK_DIR};$ENV{PATH}")
    endif ()
endforeach ()

set(functions_framework_cpp_integration_programs
    # cmake-format: sort
    cloud_event_conformance.cc cloud_event_handler.cc echo_server.cc
    http_conformance.cc)

foreach (fname ${functions_framework_cpp_integration_programs})
    string(REPLACE "/" "_" target "${fname}")
    string(REPLACE ".cc" "" target "${target}")
    add_executable("${target}" ${fname})
    target_link_libraries(${target} PRIVATE functions-framework-cpp::framework)
    functions_framework_cpp_add_common_options(${target})
endforeach ()
