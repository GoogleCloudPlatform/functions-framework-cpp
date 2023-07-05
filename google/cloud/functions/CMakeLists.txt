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
    functions_framework_cpp # cmake-format: sort
    ${CMAKE_CURRENT_BINARY_DIR}/internal/build_info.cc
    cloud_event.cc
    cloud_event.h
    framework.h
    function.cc
    function.h
    http_request.h
    http_response.cc
    http_response.h
    internal/base64_decode.cc
    internal/base64_decode.h
    internal/build_info.h
    internal/call_user_function.cc
    internal/call_user_function.h
    internal/compiler_info.cc
    internal/compiler_info.h
    internal/framework_impl.cc
    internal/framework_impl.h
    internal/function_impl.cc
    internal/function_impl.h
    internal/http_message_types.h
    internal/parse_cloud_event_http.cc
    internal/parse_cloud_event_http.h
    internal/parse_cloud_event_json.cc
    internal/parse_cloud_event_json.h
    internal/parse_cloud_event_legacy.cc
    internal/parse_cloud_event_legacy.h
    internal/parse_cloud_event_storage.cc
    internal/parse_cloud_event_storage.h
    internal/parse_options.cc
    internal/parse_options.h
    internal/setenv.cc
    internal/setenv.h
    internal/version_info.h
    internal/wrap_request.cc
    internal/wrap_request.h
    internal/wrap_response.cc
    internal/wrap_response.h
    user_functions.h
    version.cc
    version.h)
functions_framework_cpp_add_common_options(functions_framework_cpp)
if (MSVC)
    set_property(
        SOURCE internal/parse_options.cc
        APPEND
        PROPERTY COMPILE_DEFINITIONS "_CRT_SECURE_NO_WARNINGS")
endif ()
target_include_directories(functions_framework_cpp
                           PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>)
target_include_directories(functions_framework_cpp SYSTEM
                           PUBLIC $<INSTALL_INTERFACE:include>)
target_link_libraries(
    functions_framework_cpp PUBLIC absl::time Boost::headers
                                   Boost::program_options Threads::Threads)
set_target_properties(
    functions_framework_cpp
    PROPERTIES EXPORT_NAME functions-framework-cpp::framework
               VERSION "${PROJECT_VERSION}"
               SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
add_library(functions-framework-cpp::framework ALIAS functions_framework_cpp)

if (BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)
    set(functions_framework_cpp_unit_tests
        # cmake-format: sort
        cloud_event_test.cc
        http_request_test.cc
        http_response_test.cc
        internal/base64_decode_test.cc
        internal/call_user_function_test.cc
        internal/compiler_info_test.cc
        internal/framework_impl_test.cc
        internal/function_impl_test.cc
        internal/parse_cloud_event_http_test.cc
        internal/parse_cloud_event_json_test.cc
        internal/parse_cloud_event_legacy_test.cc
        internal/parse_cloud_event_storage_test.cc
        internal/parse_options_test.cc
        internal/wrap_request_test.cc
        version_test.cc)

    foreach (fname ${functions_framework_cpp_unit_tests})
        string(REPLACE "/" "_" target "${fname}")
        string(REPLACE ".cc" "" target "${target}")
        add_executable("${target}" ${fname})
        target_link_libraries(
            ${target}
            PRIVATE functions-framework-cpp::framework GTest::gmock_main
                    GTest::gmock GTest::gtest Boost::headers)
        functions_framework_cpp_add_common_options(${target})
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
    TARGETS functions_framework_cpp
    EXPORT functions-framework-cpp-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT runtime
            NAMELINK_SKIP
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT development
            NAMELINK_COMPONENT development
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT development)

google_cloud_cpp_install_headers(functions_framework_cpp
                                 include/google/cloud/functions)

# Create and install the CMake configuration files.
install(EXPORT functions-framework-cpp-targets
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/functions_framework_cpp")
configure_package_config_file(
    "config.cmake.in" "functions_framework_cpp-config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/functions_framework_cpp"
    NO_SET_AND_CHECK_MACRO)
write_basic_package_version_file(
    "functions_framework_cpp-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMinorVersion)
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/functions_framework_cpp-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/functions_framework_cpp-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/functions_framework_cpp")

# Create and install the pkg-config configuration files.
configure_file("config.pc.in" "functions_framework_cpp.pc" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/functions_framework_cpp.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
