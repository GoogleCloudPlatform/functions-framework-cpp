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

#ifndef FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_HTTP_REQUEST_H
#define FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_HTTP_REQUEST_H

#include "google/cloud/functions/version.h"
#include <map>
#include <memory>
#include <string>

namespace google::cloud::functions {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {

/**
 * Represents a HTTP request.
 *
 * Functions to handle HTTP requests receive an object of this class.
 */
class HttpRequest {
 public:
  using HeadersType = std::multimap<std::string, std::string>;

  /// The HTTP verb (GET, PUT, POST, etc) in the request
  [[nodiscard]] std::string const& verb() const { return impl_->verb(); }

  /// The target object for the request, e.g, `/index.html`.
  [[nodiscard]] std::string const& target() const { return impl_->target(); }

  /// The request payload
  [[nodiscard]] std::string const& payload() const { return impl_->payload(); }

  /// The request HTTP headers
  [[nodiscard]] HeadersType const& headers() const { return impl_->headers(); }

  /// The HTTP version for the request
  [[nodiscard]] int version_major() const { return impl_->version_major(); }
  [[nodiscard]] int version_minor() const { return impl_->version_minor(); }

  class Impl {
   public:
    virtual ~Impl() = 0;
    [[nodiscard]] virtual std::string const& verb() const = 0;
    [[nodiscard]] virtual std::string const& target() const = 0;
    [[nodiscard]] virtual std::string const& payload() const = 0;
    [[nodiscard]] virtual HeadersType const& headers() const = 0;
    [[nodiscard]] virtual int version_major() const = 0;
    [[nodiscard]] virtual int version_minor() const = 0;
  };

  explicit HttpRequest(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}

 private:
  std::shared_ptr<Impl> impl_;
};

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions

#endif  // FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_HTTP_REQUEST_H
