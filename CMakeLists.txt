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

cmake_minimum_required(VERSION 3.10)

if (NOT (CMAKE_VERSION VERSION_LESS "3.14"))
    cmake_policy(SET CMP0091 NEW)
endif ()

option(BUILD_TESTING "Enable tests" ON)
if (BUILD_TESTING)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif ()
option(FUNCTIONS_FRAMEWORK_CPP_TEST_EXAMPLES "Enable testing for examples" ON)
mark_as_advanced(FUNCTIONS_FRAMEWORK_CPP_TEST_EXAMPLES)
if (FUNCTIONS_FRAMEWORK_CPP_TEST_EXAMPLES)
    list(APPEND VCPKG_MANIFEST_FEATURES "examples")
endif ()

set(PACKAGE_BUGREPORT
    "http://github.com/GoogleCloudPlatform/functions-framework-cpp")
project(
    functions-framework-cpp
    VERSION 1.1.0
    DESCRIPTION "Functions Framework for C++"
    LANGUAGES CXX)

# Configure the compiler options, we will be using C++17 features.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

if (VCPKG_TARGET_TRIPLET MATCHES "-static$")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else ()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif ()

include(CTest)
include(FunctionsFrameworkCppHelpers)
add_subdirectory(google/cloud/functions)

if (BUILD_TESTING AND FUNCTIONS_FRAMEWORK_CPP_TEST_EXAMPLES)
    add_subdirectory(examples)
endif ()
