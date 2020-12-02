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

find_package(absl REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)
find_package(Threads REQUIRED)

add_library(
    googleapis_functions_framework # cmake-format: sort
    ${CMAKE_CURRENT_BINARY_DIR}/internal/build_info.cc
    cloud_event.cc
    cloud_event.h
    http_request.cc
    http_request.h
    http_response.cc
    http_response.h
    internal/build_info.h
    internal/call_user_function.cc
    internal/call_user_function.h
    internal/compiler_info.cc
    internal/compiler_info.h
    internal/framework.cc
    internal/framework.h
    internal/http_message_types.h
    internal/parse_cloud_event_http.cc
    internal/parse_cloud_event_http.h
    internal/parse_cloud_event_json.cc
    internal/parse_cloud_event_json.h
    internal/parse_options.cc
    internal/parse_options.h
    internal/user_functions.h
    internal/version_info.h
    internal/wrap_request.cc
    internal/wrap_request.h
    internal/wrap_response.cc
    internal/wrap_response.h
    version.cc
    version.h)
target_include_directories(googleapis_functions_framework
                           PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>)

target_include_directories(googleapis_functions_framework SYSTEM
                           PUBLIC $<INSTALL_INTERFACE:include>)
target_link_libraries(
    googleapis_functions_framework
    PUBLIC absl::time Boost::headers Boost::program_options Threads::Threads)
target_compile_definitions(googleapis_functions_framework
                           PUBLIC BOOST_BEAST_USE_STD_STRING_VIEW)
set_target_properties(
    googleapis_functions_framework
    PROPERTIES EXPORT_NAME googleapis-c++::functions_framework)
add_library(googleapis-c++::functions_framework ALIAS
            googleapis_functions_framework)

if (BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)
    set(googleapis_functions_framework_unit_tests
        # cmake-format: sort
        cloud_event_test.cc
        http_response_test.cc
        internal/call_user_function_test.cc
        internal/compiler_info_test.cc
        internal/framework_test.cc
        internal/parse_cloud_event_http_test.cc
        internal/parse_cloud_event_json_test.cc
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

include(InstallHeaders)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# Install the libraries and headers in the locations determined by
# GNUInstallDirs
install(
    TARGETS googleapis_functions_framework
    EXPORT googleapis_functions_framework-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT runtime
            NAMELINK_SKIP
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT development
            NAMELINK_COMPONENT development
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT development)

google_cloud_cpp_install_headers(googleapis_functions_framework
                                 include/google/cloud/functions)

# Create and install the CMake configuration files.
install(
    EXPORT googleapis_functions_framework-targets
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/googleapis_functions_framework")
configure_file("config.cmake.in" "googleapis_functions_framework-config.cmake"
               @ONLY)
write_basic_package_version_file(
    "googleapis_functions_framework-config-version.cmake"
    VERSION ${FUNCTIONS_FRAMEWORK_CPP_VERSION}
    COMPATIBILITY SameMinorVersion)
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/googleapis_functions_framework-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/googleapis_functions_framework-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/googleapis_functions_framework")

# Create and install the pkg-config configuration files.
configure_file("config.pc.in" "googleapis_functions_framework.pc" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/googleapis_functions_framework.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
