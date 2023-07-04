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
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_BEGIN

/**
 * Represents an HTTP request.
 *
 * Functions to handle HTTP requests receive an object of this type.
 */
class HttpRequest {
 public:
  using HeadersType = std::multimap<std::string, std::string>;

  HttpRequest() = default;

  /// The HTTP verb (GET, PUT, POST, etc) in the request
  [[nodiscard]] std::string const& verb() const { return verb_; }

  /// The target object for the request, e.g, `/index.html`.
  [[nodiscard]] std::string const& target() const { return target_; }

  /// The request payload
  [[nodiscard]] std::string const& payload() const& { return payload_; }
  [[nodiscard]] std::string&& payload() && { return std::move(payload_); }

  /// The request HTTP headers
  [[nodiscard]] HeadersType const& headers() const { return headers_; }

  /// The HTTP version for the request
  [[nodiscard]] int version_major() const { return version_major_; }
  [[nodiscard]] int version_minor() const { return version_minor_; }

  HttpRequest& set_verb(std::string v) & {
    verb_ = std::move(v);
    return *this;
  }
  HttpRequest&& set_verb(std::string v) && {
    return std::move(set_verb(std::move(v)));
  }

  HttpRequest& set_target(std::string v) & {
    target_ = std::move(v);
    return *this;
  }
  HttpRequest&& set_target(std::string v) && {
    return std::move(set_target(std::move(v)));
  }

  HttpRequest& set_payload(std::string v) & {
    payload_ = std::move(v);
    return *this;
  }
  HttpRequest&& set_payload(std::string v) && {
    return std::move(set_payload(std::move(v)));
  }

  HttpRequest& clear_headers() & {
    headers_.clear();
    return *this;
  }
  HttpRequest&& clear_headers() && { return std::move(clear_headers()); }

  HttpRequest& add_header(std::string k, std::string v) & {
    headers_.emplace(std::move(k), std::move(v));
    return *this;
  }
  HttpRequest&& add_header(std::string k, std::string v) && {
    return std::move(add_header(std::move(k), std::move(v)));
  }

  HttpRequest& remove_header(std::string const& k) & {
    headers_.erase(k);
    return *this;
  }
  HttpRequest&& remove_header(std::string const& k) && {
    return std::move(remove_header(k));
  }

  HttpRequest& set_version(int major, int minor) & {
    version_major_ = major;
    version_minor_ = minor;
    return *this;
  }
  HttpRequest&& set_version(int major, int minor) && {
    return std::move(set_version(major, minor));
  }

 private:
  std::string verb_;
  std::string target_;
  std::string payload_;
  HeadersType headers_;
  int version_major_ = 1;
  int version_minor_ = 1;
};

FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_END
}  // namespace google::cloud::functions

#endif  // FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_HTTP_REQUEST_H
