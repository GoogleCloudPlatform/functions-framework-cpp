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

#include "google/cloud/functions/internal/wrap_request.h"

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {
namespace {
class WrapBeastImpl : public google::cloud::functions::HttpRequest::Impl {
 public:
  using Headers = google::cloud::functions::HttpRequest::Headers;

  explicit WrapBeastImpl(BeastRequest request)
      : verb_(request.method_string()), target_(request.target()) {
    for (auto const& h : request.base()) {
      headers_.emplace(h.name_string(), h.value());
    }
    version_ = static_cast<int>(request.version());
    // We copy all the other values before moving `body()`.
    body_ = std::move(request).body();
  }
  ~WrapBeastImpl() override = default;

  [[nodiscard]] std::string const& verb() const override { return verb_; }
  [[nodiscard]] std::string const& target() const override { return target_; }
  [[nodiscard]] std::string const& payload() const override { return body_; }
  [[nodiscard]] Headers const& headers() const override { return headers_; }
  static inline auto constexpr kBeastHttpVersionFactor = 10;
  [[nodiscard]] int version_major() const override {
    return version_ / kBeastHttpVersionFactor;
  }
  [[nodiscard]] int version_minor() const override {
    return version_ % kBeastHttpVersionFactor;
  }

 private:
  // Hold the components of the original `BeastRequest` object. We cannot hold
  // the object directly and use its accessors because they return
  // `std::string_view`, and we want to return `std::string const&`.
  std::string verb_;
  std::string target_;
  std::string body_;
  Headers headers_;
  int version_;
};
}  // namespace

::google::cloud::functions::HttpRequest FromBeast(BeastRequest request) {
  return functions::HttpRequest(
      std::make_unique<WrapBeastImpl>(std::move(request)));
}

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal
