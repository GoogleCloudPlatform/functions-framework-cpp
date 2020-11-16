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

include(BuildMetadata)
configure_file(internal/build_info.cc.in internal/build_info.cc)

configure_file(internal/version_info.h.in
               ${CMAKE_CURRENT_SOURCE_DIR}/internal/version_info.h)

find_package(Boost REQUIRED COMPONENTS program_options)
find_package(Threads REQUIRED)

add_library(
    googleapis_functions_framework # cmake-format: sort
    ${CMAKE_CURRENT_BINARY_DIR}/internal/build_info.cc
    framework.cc
    framework.h
    http_request.cc
    http_request.h
    http_response.cc
    http_response.h
    internal/build_info.h
    internal/call_user_function.cc
    internal/call_user_function.h
    internal/compiler_info.cc
    internal/compiler_info.h
    internal/framework_impl.cc
    internal/framework_impl.h
    internal/http_message_types.h
    internal/parse_options.cc
    internal/parse_options.h
    internal/version_info.h
    internal/wrap_request.cc
    internal/wrap_request.h
    internal/wrap_request_test.cc
    internal/wrap_response.cc
    internal/wrap_response.h
    version.cc
    version.h)
target_include_directories(googleapis_functions_framework
                           PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(
    googleapis_functions_framework PUBLIC Boost::headers Boost::program_options
                                          Threads::Threads)
target_compile_definitions(googleapis_functions_framework
                           PUBLIC BOOST_BEAST_USE_STD_STRING_VIEW)

if (BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)
    set(googleapis_functions_framework_unit_tests
        # cmake-format: sort
        http_response_test.cc
        internal/call_user_function_test.cc
        internal/compiler_info_test.cc
        internal/framework_test.cc
        internal/parse_options_test.cc
        internal/wrap_request_test.cc
        version_test.cc)

    foreach (fname ${googleapis_functions_framework_unit_tests})
        string(REPLACE "/" "_" target "${fname}")
        string(REPLACE ".cc" "" target "${target}")
        add_executable("${target}" ${fname})
        target_link_libraries(
            ${target} PRIVATE googleapis_functions_framework GTest::gmock_main
                              GTest::gmock GTest::gtest)
        add_test(NAME ${target} COMMAND ${target})
    endforeach ()

    add_subdirectory(integration_tests)
endif ()
