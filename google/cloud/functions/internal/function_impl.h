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

#ifndef FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_INTERNAL_FUNCTION_IMPL_H
#define FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_INTERNAL_FUNCTION_IMPL_H

#include "google/cloud/functions/internal/http_message_types.h"
#include "google/cloud/functions/user_functions.h"
#include "google/cloud/functions/version.h"
#include <map>
#include <string_view>

namespace google::cloud::functions {
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_BEGIN
class Function;
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_END
}  // namespace google::cloud::functions

namespace google::cloud::functions_internal {
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_BEGIN

using Handler = std::function<BeastResponse(BeastRequest)>;

class FunctionImpl {
 public:
  virtual ~FunctionImpl() = default;
  [[nodiscard]] virtual Handler GetHandler(std::string_view target) const = 0;

  static std::shared_ptr<FunctionImpl> GetImpl(functions::Function const& fun);
  static functions::Function MakeFunction(std::shared_ptr<FunctionImpl> impl);
};

class BaseFunctionImpl : public FunctionImpl {
 public:
  explicit BaseFunctionImpl(functions::UserHttpFunction handler);
  explicit BaseFunctionImpl(functions::UserCloudEventFunction handler);
  ~BaseFunctionImpl() override = default;

  [[nodiscard]] Handler GetHandler(std::string_view /*target*/) const override {
    return handler_;
  }

 private:
  Handler handler_;
};

class MapFunctionImpl : public FunctionImpl {
 public:
  explicit MapFunctionImpl(std::map<std::string, functions::Function> mapping);
  ~MapFunctionImpl() override = default;

  [[nodiscard]] Handler GetHandler(std::string_view target) const override;

 private:
  std::map<std::string, functions::Function> mapping_;
};

FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_END
}  // namespace google::cloud::functions_internal

#endif  // FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_INTERNAL_FUNCTION_IMPL_H
