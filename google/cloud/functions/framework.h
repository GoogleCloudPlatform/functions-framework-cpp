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

#ifndef FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_FRAMEWORK_H
#define FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_FRAMEWORK_H

#include "google/cloud/functions/function.h"
#include "google/cloud/functions/user_functions.h"
#include "google/cloud/functions/version.h"
#include <functional>

namespace google::cloud::functions {
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_BEGIN

/**
 * Runs function wrapped by @p handler.
 *
 * Starts a HTTP server at the address and listening endpoint described by
 * @p argv, invoking @p handler to handle any HTTP request.
 *
 * If @p handler wraps a function with the Cloud Event signature, then the
 * incoming HTTP requests **MUST** conform to the Cloud Events [HTTP protocol
 * binding specification][cloud-events-spec].
 *
 * @note
 * When deploying code to Google Cloud Functions applications should **not** use
 * this function directly. The buildpack will automatically create a `main()`
 * and invoke `Run()` with the correct parameters. We recommend that application
 * developers use this function only for local development and integration
 * tests.
 *
 * @par Example
 * @code
 * namespace gcf = ::google::cloud::functions;
 *
 * extern gcf::HttpResponse MyHandler(gcf::HttpRequest);
 *
 * int main(int argc, char* argv[]) {
 *   return gcf::Run(argc, argv, gcf::MakeFunction(MyHandler));
 * }
 * @endcode
 *
 * @see ParseOptions for more details of the command-line arguments used by this
 *     function.
 *
 * [cloud-events-spec]:
 * https://github.com/cloudevents/spec/blob/v1.0/http-protocol-binding.md
 */
int Run(int argc, char const* const argv[], Function const& handler) noexcept;

/**
 * Run the given function, invoking it to handle HTTP requests.
 *
 * @deprecated Prefer using the overload consuming a `Function` object.
 *
 * Starts a HTTP server at the address and listening endpoint described by
 * @p argv, invoking @p handler to handle any HTTP request.
 *
 * When deploying code to Google Cloud Functions applications should **not** use
 * this function directly. The buildpack will automatically create a `main()`
 * and invoke `Run()` with the correct parameters. We recommend that application
 * developers use this function only for local development and integration
 * tests.
 *
 * @par Example
 * @code
 * namespace gcf = ::google::cloud::functions;
 *
 * extern gcf::HttpResponse MyHandler(gcf::HttpRequest);
 *
 * int main(int argc, char* argv[]) {
 *   return gcf::Run(argc, argv, MyHandler);
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
 * @deprecated Prefer using the overload consuming a `Function` object.
 *
 * Starts a HTTP server at the address and listening endpoint described by
 * @p argv, invoking @p handler to handle any HTTP request which *MUST* conform
 * to the Cloud Events [HTTP protocol binding][cloud-events-spec].
 *
 * When deploying code to Google Cloud Functions applications should **not** use
 * this function directly. The buildpack will automatically create a `main()`
 * and invoke `Run()` with the correct parameters. We recommend that application
 * developers use this function only for local development and integration
 * tests.
 *
 * @par Example
 * @code
 * namespace gcf = ::google::cloud::functions;
 *
 * extern void MyHandler(gcf::CloudEvent);
 *
 * int main(int argc, char* argv[]) {
 *   return gcf::Run(argc, argv, MyHandler);
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

FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_END
}  // namespace google::cloud::functions

#endif  // FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_FRAMEWORK_H
