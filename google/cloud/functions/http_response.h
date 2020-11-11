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

#ifndef FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_HTTP_RESPONSE_H
#define FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_HTTP_RESPONSE_H

#include "google/cloud/functions/version.h"
#include <map>
#include <memory>
#include <string>
#include <string_view>

namespace google::cloud::functions {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {

/**
 * Represents a HTTP request.
 *
 * Functions to handle HTTP requests receive an object of this class.
 */
class HttpResponse {
 public:
  using HeadersType = std::map<std::string, std::string>;

  HttpResponse();

  /// The request payload
  void payload(std::string v) { impl_->payload(std::move(v)); }
  [[nodiscard]] std::string const& payload() const { return impl_->payload(); }

  /// Set the status result
  void result(int code) { impl_->result(code); }
  [[nodiscard]] int result() const { return impl_->result(); }

  /// The request HTTP headers
  void header(std::string_view name, std::string_view value) {
    impl_->header(name, value);
  }
  [[nodiscard]] HeadersType headers() const { return impl_->headers(); }

  /// The HTTP version for the request
  void version(int major, int minor) { impl_->version(major, minor); }
  [[nodiscard]] int version_major() const { return impl_->version_major(); }
  [[nodiscard]] int version_minor() const { return impl_->version_minor(); }

  class Impl {
   public:
    virtual ~Impl() = 0;
    virtual void payload(std::string) = 0;
    [[nodiscard]] virtual std::string const& payload() const = 0;
    virtual void result(int code) = 0;
    [[nodiscard]] virtual int result() const = 0;
    virtual void header(std::string_view name, std::string_view value) = 0;
    [[nodiscard]] virtual HeadersType headers() const = 0;
    virtual void version(int major, int minor) = 0;
    [[nodiscard]] virtual int version_major() const = 0;
    [[nodiscard]] virtual int version_minor() const = 0;
  };

  explicit HttpResponse(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}

 private:
  std::shared_ptr<Impl> impl_;
};

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions

#endif  // FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_HTTP_RESPONSE_H
