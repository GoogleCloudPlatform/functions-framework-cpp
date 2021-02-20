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

# Define a function to fetch the current git revision. Using a function creates
# a new scope, so the CMake variables do not leak to the global namespace.
function (functions_framework_cpp_initialize_git_head var)
    set(result "")
    # If we cannot find a `.git` directory do not even try to guess the git
    # revision.
    if (IS_DIRECTORY ${PROJECT_SOURCE_DIR}/.git)
        set(result "unknown-commit")
        # We need `git` to find the revision.
        find_program(FUNCTIONS_FRAMEWORK_CPP_GIT_PROGRAM NAMES git)
        mark_as_advanced(FUNCTIONS_FRAMEWORK_CPP_GIT_PROGRAM)
        if (FUNCTIONS_FRAMEWORK_CPP_GIT_PROGRAM)
            # Run `git rev-parse --short HEAD` and capture the output in a
            # variable.
            execute_process(
                COMMAND "${FUNCTIONS_FRAMEWORK_CPP_GIT_PROGRAM}" rev-parse
                        --short HEAD
                OUTPUT_VARIABLE GIT_HEAD_LOG
                ERROR_VARIABLE GIT_HEAD_LOG)
            string(REPLACE "\n" "" result "${GIT_HEAD_LOG}")
        endif ()
    endif ()
    set(${var}
        "${result}"
        PARENT_SCOPE)
    message(STATUS "functions-framework-cpp build metadata set to ${result}")
endfunction ()

# Capture the compiler version and the git revision into variables, then
# generate a config file with the values.
if (NOT ("${FUNCTIONS_FRAMEWORK_CPP_BUILD_METADATA}" STREQUAL ""))
    # The build metadata flag is already defined, do not re-compute the
    # initialization value. This works both when the user supplies
    # -DFUNCTIONS_FRAMEWORK_CPP_METADATA=value in the command line, and when
    # FUNCTIONS_FRAMEWORK_CPP_METADATA has a cached value
    set(FUNCTIONS_FRAMEWORK_CPP_GIT_HEAD "unused")
else ()
    functions_framework_cpp_initialize_git_head(
        FUNCTIONS_FRAMEWORK_CPP_GIT_HEAD)
endif ()

# Define a CMake configuration option to set the build metadata. By default this
# is initialized from `git rev-parse --short HEAD`, but the developer (or the
# script building via CMake) can override the value.
set(FUNCTIONS_FRAMEWORK_CPP_BUILD_METADATA
    "${FUNCTIONS_FRAMEWORK_CPP_GIT_HEAD}"
    CACHE STRING "Append build metadata to the library version number")
# This option is rarely needed. Mark it as "advanced" to remove it from the
# default CMake UIs.
mark_as_advanced(FUNCTIONS_FRAMEWORK_CPP_BUILD_METADATA)
