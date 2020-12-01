// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_INTERNAL_FRAMEWORK_H
#define FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_INTERNAL_FRAMEWORK_H

#include "google/cloud/functions/internal/user_functions.h"
#include "google/cloud/functions/version.h"
#include <functional>

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {

/**
 * Run the given function, invoking it to handle HTTP requests.
 *
 * Starts a HTTP server at the address and listening endpoint described by
 * @p argv, invoking @p handler to handle any HTTP request. Note that we do not
 * expect the application to use this function directly (it is in
 * `functions_internal` after all), instead, our build scripts should create
 * a `main()` function that calls this, as shown in the example:
 *
 * @par Example
 * @code
 * using ::google::cloud::functions_internal::Run;
 *
 * HttpFunction FindHttpHandler() { return some_user_function; }
 *
 * int main(int argc, char* argv[]) {
 *   return Run(argc, argv, FindHttpHandler());
 * }
 * @endcode
 *
 * @see ParseOptions for more details of the command-line arguments used by this
 *     function.
 */
int Run(int argc, char const* const argv[], UserHttpFunction handler) noexcept;

/**
 * Run the given function, invoking it to handle Cloud Events.
 *
 * Starts a HTTP server at the address and listening endpoint described by
 * @p argv, invoking @p handler to handle any HTTP request which *MUST* conform
 * to the Cloud Events [HTTP protocol binding][cloud-events-spec]. Note that we
 * do not expect the application to use this function directly (it is in
 * `functions_internal` after all), instead, our build scripts should create a
 * `main()` function that calls this, as shown in the example:
 *
 * @par Example
 * @code
 * using ::google::cloud::functions_internal::Run;
 *
 * CloudEventFunction FindCloudEventHandler() { return some_user_function; }
 *
 * int main(int argc, char* argv[]) {
 *   return Run(argc, argv, FindHttpHandler());
 * }
 * @endcode
 *
 * @see ParseOptions for more details of the command-line arguments used by this
 *     function.
 *
 * [cloud-events-spec]:
 * https://github.com/cloudevents/spec/blob/v1.0/http-protocol-binding.md
 */
int Run(int argc, char const* const argv[],
        UserCloudEventFunction handler) noexcept;

/// Implement functions::Run(), with additional helpers for testing.
int RunForTest(int argc, char const* const argv[], UserHttpFunction handler,
               std::function<bool()> const& shutdown,
               std::function<void(int)> const& actual_port);

/// Implement functions::Run(), with additional helpers for testing.
int RunForTest(int argc, char const* const argv[],
               UserCloudEventFunction handler,
               std::function<bool()> const& shutdown,
               std::function<void(int)> const& actual_port);

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal

#endif  // FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_INTERNAL_FRAMEWORK_H
