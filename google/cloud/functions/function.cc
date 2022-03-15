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

#include "google/cloud/functions/function.h"
#include "google/cloud/functions/internal/function_impl.h"

namespace google::cloud::functions {
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_BEGIN

Function::~Function() = default;

Function::Function(std::shared_ptr<functions_internal::FunctionImpl> impl)
    : impl_(std::move(impl)) {}

Function MakeFunction(UserHttpFunction function) {
  return functions_internal::FunctionImpl::MakeFunction(
      std::make_shared<functions_internal::BaseFunctionImpl>(
          std::move(function)));
}

Function MakeFunction(UserCloudEventFunction function) {
  return functions_internal::FunctionImpl::MakeFunction(
      std::make_shared<functions_internal::BaseFunctionImpl>(
          std::move(function)));
}

Function MakeFunction(std::map<std::string, Function> mapping) {
  return functions_internal::FunctionImpl::MakeFunction(
      std::make_shared<functions_internal::MapFunctionImpl>(
          std::move(mapping)));
}

FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_END
}  // namespace google::cloud::functions
