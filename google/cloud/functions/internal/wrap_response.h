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

#ifndef FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_INTERNAL_WRAP_RESPONSE_H
#define FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_INTERNAL_WRAP_RESPONSE_H

#include "google/cloud/functions/internal/http_message_types.h"
#include "google/cloud/functions/http_response.h"
#include "google/cloud/functions/version.h"

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {

struct UnwrapResponse;

class WrapResponseImpl : public google::cloud::functions::HttpResponse::Impl {
 public:
  using Headers = google::cloud::functions::HttpResponse::HeadersType;

  WrapResponseImpl() = default;
  ~WrapResponseImpl() override = default;

  void set_payload(std::string v) override { response_.body() = std::move(v); }
  [[nodiscard]] std::string const& payload() const override {
    return response_.body();
  }
  void set_result(int code) override { response_.result(code); }
  [[nodiscard]] int result() const override {
    return static_cast<int>(response_.result_int());
  }

  void set_header(std::string_view name, std::string_view value) override {
    response_.set(name, value);
  }
  [[nodiscard]] Headers headers() const override {
    Headers h;
    for (auto const& f : response_) {
      h.emplace(f.name_string(), f.value());
    }
    return h;
  }

  static inline auto constexpr kBeastHttpVersionFactor = 10;
  void set_version(int major, int minor) override {
    response_.version(major * kBeastHttpVersionFactor + minor);
  }
  [[nodiscard]] int version_major() const override {
    return static_cast<int>(response_.version()) / kBeastHttpVersionFactor;
  }
  [[nodiscard]] int version_minor() const override {
    return static_cast<int>(response_.version()) % kBeastHttpVersionFactor;
  }

 private:
  friend struct UnwrapResponse;
  BeastResponse response_;
};

/// Wrap a Boost.Beast request into a functions framework HTTP request.
std::shared_ptr<functions::HttpResponse::Impl> MakeHttpResponse();

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal

#endif  // FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_INTERNAL_WRAP_RESPONSE_H
