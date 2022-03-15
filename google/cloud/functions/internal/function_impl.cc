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

#include "google/cloud/functions/internal/function_impl.h"
#include "google/cloud/functions/internal/call_user_function.h"
#include "google/cloud/functions/function.h"

namespace google::cloud::functions_internal {
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_BEGIN

std::shared_ptr<FunctionImpl> FunctionImpl::GetImpl(
    functions::Function const& fun) {
  return fun.impl_;
}

functions::Function FunctionImpl::MakeFunction(
    std::shared_ptr<FunctionImpl> impl) {
  return functions::Function(std::move(impl));
}

BaseFunctionImpl::BaseFunctionImpl(functions::UserHttpFunction function)
    : handler_([fun = std::move(function)](BeastRequest request) {
        return CallUserFunction(fun, std::move(request));
      }) {}

BaseFunctionImpl::BaseFunctionImpl(functions::UserCloudEventFunction function)
    : handler_([fun = std::move(function)](BeastRequest const& request) {
        return CallUserFunction(fun, request);
      }) {}

[[nodiscard]] Handler BaseFunctionImpl::GetHandler(
    std::string_view /*target*/) const {
  return handler_;
}

MapFunctionImpl::MapFunctionImpl(
    std::map<std::string, functions::Function> mapping)
    : mapping_(std::move(mapping)) {}

[[nodiscard]] Handler MapFunctionImpl::GetHandler(
    std::string_view target) const {
  auto const l = mapping_.find(std::string(target));
  if (l == mapping_.end()) {
    throw std::runtime_error("Function not found " + std::string(target));
  }
  return FunctionImpl::GetImpl(l->second)->GetHandler(target);
}

FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_END
}  // namespace google::cloud::functions_internal
