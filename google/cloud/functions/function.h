// Copyright 2022 Google LLC
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

#ifndef FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_FUNCTION_H
#define FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_FUNCTION_H

#include "google/cloud/functions/user_functions.h"
#include "google/cloud/functions/version.h"
#include <map>

namespace google::cloud::functions_internal {
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_BEGIN
class FunctionImpl;
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_END
}  // namespace google::cloud::functions_internal

namespace google::cloud::functions {
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_BEGIN

/**
 * An opaque wrapper around user-defined `http` and `cloud event` functions.
 *
 * This class can hold any of the function types supported by the C++ functions
 * framework.  The framework provides a number of factory functions to create
 * `Function` objects from application defined functions.
 *
 * When using the functions framework for local development applications can
 * create a HTTP server using:
 *
 * @code
 * namespace gcf = google::cloud::functions;
 * auto MyFunction() { return gcf::MakeFunction(callable); }
 * int main(int argc, char* argv[]) {
 *   return gcf::Run(argc, argv, MyFunction());
 * }
 * @endcode
 *
 * When using one of the buildpacks, the application defines the factory
 * function and references it (by name) in the GOOGLE_FUNCTION_TARGET setting:
 *
 * @code
 * namespace my_namespace {
 * gcf::HttpResponse Handler(gcf::Request const&) { ... code here ... }
 *
 * // Use by setting GOOGLE_FUNCTION_TARGET to "my_namespace::MyFunction"
 * auto MyFunction() { return gcf::MakeFunction(Handler); }
 * } //
 * @endcode
 */
class Function {
 public:
  ~Function();

  friend bool operator==(Function const& lhs, Function const& rhs) {
    return lhs.impl_ == rhs.impl_;
  }
  friend bool operator!=(Function const& lhs, Function const& rhs) {
    return !(lhs == rhs);
  }

 private:
  friend class functions_internal::FunctionImpl;

  explicit Function(std::shared_ptr<functions_internal::FunctionImpl> impl);
  std::shared_ptr<functions_internal::FunctionImpl> impl_;
};

/// Wraps a `http` handler.
Function MakeFunction(UserHttpFunction function);

/// Wraps a `cloud event` handler.
Function MakeFunction(UserCloudEventFunction function);

/**
 * Creates a function with support for runtime-assigned targets.
 *
 * Given a map from function names to `Function` objects it creates a new
 * function that will, at runtime, retrieve the correct function from the map,
 * and install it as the handler. This assignment happens during function
 * startup.
 */
Function MakeFunction(std::map<std::string, Function> mapping);

FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_END
}  // namespace google::cloud::functions

#endif  // FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_FUNCTION_H
