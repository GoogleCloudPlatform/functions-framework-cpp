# ~~~
# Copyright 2021 Google LLC
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

# A minimal CMakeList.txt showing how to use the Functions Framework for C++ in
# CMake-based projects.

cmake_minimum_required(VERSION 3.5)
project(functions-framework-cpp-howto-local-development CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(functions_framework_cpp REQUIRED)

add_executable(local_server local_server.cc)
target_link_libraries(local_server functions-framework-cpp::framework)
