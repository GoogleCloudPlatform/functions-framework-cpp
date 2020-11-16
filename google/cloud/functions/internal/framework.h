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

#include "google/cloud/functions/http_request.h"
#include "google/cloud/functions/http_response.h"
#include "google/cloud/functions/version.h"
#include <functional>

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {

using HttpFunction =
    std::function<functions::HttpResponse(functions::HttpRequest)>;

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
 * using ::google::cloud::functions::internal::Run;
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
int Run(int argc, char const* const argv[], HttpFunction handler) noexcept;

/// Implement functions::Run(), with additional helpers for testing.
int RunForTest(int argc, char const* const argv[], HttpFunction handler,
               std::function<bool()> const& shutdown,
               std::function<void(int)> const& actual_port);

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal

#endif  // FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_INTERNAL_FRAMEWORK_H
