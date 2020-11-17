# Copyright 2020 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

cmake_minimum_required(VERSION 3.10)
project(functions-framework-application CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(googleapis_functions_framework)

function (functions_framework_cpp_define_target_with_glob directory)
    file(GLOB_RECURSE application_sources "${directory}/*.cc"
         "${directory}/*.cpp" "${directory}/*.cxx")
    add_library(functions_framework_cpp_function ${application_sources})
endfunction ()

if (EXISTS "application/CMakeLists.txt")
    add_subdirectory(application)
else ()
    functions_framework_cpp_define_target_with_glob("application")
endif ()

if (NOT TARGET functions_framework_cpp_function)
    message(
        ERROR_FATAL
        [===[No `functions_framework_cpp_function` target found

The application is expected to define a CMake library target called
`functions_framework_cpp_function` which contains the target function. The
framework automatically defines this target for you can define this target
*UNLESS* you include a top-level CMakeLists.txt file in your code. If you
do include this file you must either:
- Define the target manually, using the standard CMake commands.
- Use the the `functions_framework_cpp_define_target_with_glob` function to
  define the target.
            ]===])
endif ()

target_link_libraries(functions_framework_cpp_function
                      PUBLIC googleapis-c++::functions_framework)

add_executable(functions_framework_cpp_application main.cc)
set_target_properties(functions_framework_cpp_application
                      PROPERTIES OUTPUT_NAME application)
target_link_libraries(functions_framework_cpp_application
                      PRIVATE functions_framework_cpp_function)

install(TARGETS functions_framework_cpp_application
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
