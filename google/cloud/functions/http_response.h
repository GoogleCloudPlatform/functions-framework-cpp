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

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {
struct UnwrapResponse;
}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal

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
  HttpResponse& set_payload(std::string v) & {
    impl_->set_payload(std::move(v));
    return *this;
  }
  HttpResponse&& set_payload(std::string v) && {
    return std::move(set_payload(std::move(v)));
  }
  [[nodiscard]] std::string const& payload() const { return impl_->payload(); }

  /// Set the status result
  HttpResponse& set_result(int code) & {
    impl_->set_result(code);
    return *this;
  }
  HttpResponse&& set_result(int code) && { return std::move(set_result(code)); }
  [[nodiscard]] int result() const { return impl_->result(); }

  /// The request HTTP headers
  HttpResponse& set_header(std::string_view name, std::string_view value) & {
    impl_->set_header(name, value);
    return *this;
  }
  HttpResponse&& set_header(std::string_view name, std::string_view value) && {
    return std::move(set_header(name, value));
  }
  [[nodiscard]] HeadersType headers() const { return impl_->headers(); }

  /// The HTTP version for the request
  HttpResponse& set_version(int major, int minor) & {
    impl_->set_version(major, minor);
    return *this;
  }
  HttpResponse&& set_version(int major, int minor) && {
    return std::move(set_version(major, minor));
  }
  [[nodiscard]] int version_major() const { return impl_->version_major(); }
  [[nodiscard]] int version_minor() const { return impl_->version_minor(); }

  /**
   * @name Common HTTP status codes.
   */
  //@{
  // Information responses (1XX)
  inline static auto constexpr kContinue = 100;
  inline static auto constexpr kSwitchingProtocol = 101;
  inline static auto constexpr kProcessing = 102;
  inline static auto constexpr kEarlyHints = 103;
  // Successful responses (2XX)
  inline static auto constexpr kOkay = 200;
  inline static auto constexpr kCreated = 201;
  inline static auto constexpr kAccepted = 202;
  inline static auto constexpr kNonAuthoritativeInformation = 203;
  inline static auto constexpr kNoContent = 204;
  inline static auto constexpr kResetContent = 205;
  inline static auto constexpr kPartialContent = 206;
  inline static auto constexpr kMultiStatus = 207;
  inline static auto constexpr kAlreadyReported = 208;
  inline static auto constexpr kIMUsed = 226;
  // Redirection messages (3XX)
  inline static auto constexpr kMultipleChoice = 300;
  inline static auto constexpr kMovedPermanently = 301;
  inline static auto constexpr kFound = 302;
  inline static auto constexpr kSeeOther = 303;
  inline static auto constexpr kNotModified = 304;
  inline static auto constexpr kUseProxy = 305;
  // inline static auto constexpr kReserved = 306;
  inline static auto constexpr kTemporaryRedirect = 307;
  inline static auto constexpr kPermanentRedirect = 308;
  // Client error responses (4XX)
  inline static auto constexpr kBadRequest = 400;
  inline static auto constexpr kUnauthorized = 401;
  inline static auto constexpr kPaymentRequired = 402;
  inline static auto constexpr kForbidden = 403;
  inline static auto constexpr kNotFound = 404;
  inline static auto constexpr kMethodNotAllowed = 405;
  inline static auto constexpr kNotAcceptable = 406;
  inline static auto constexpr kProxyAuthenticationRequired = 407;
  inline static auto constexpr kRequestTimeout = 408;
  inline static auto constexpr kConflict = 409;
  inline static auto constexpr kGone = 410;
  inline static auto constexpr kLengthRequired = 411;
  inline static auto constexpr kPreconditionFailed = 412;
  inline static auto constexpr kPayloadTooLarge = 413;
  inline static auto constexpr kURITooLong = 414;
  inline static auto constexpr kUnsupportedMediaType = 415;
  inline static auto constexpr kRangeNotSatisfiable = 416;
  inline static auto constexpr kExpectationFailed = 417;
  inline static auto constexpr kTeapot = 418;
  inline static auto constexpr kMisdirectedRequest = 421;
  inline static auto constexpr kUnprocessableEntity = 422;
  inline static auto constexpr kLocked = 423;
  inline static auto constexpr kFailedDependency = 424;
  inline static auto constexpr kTooEarly = 425;
  inline static auto constexpr kUpgradeRequired = 426;
  inline static auto constexpr kPreconditionRequired = 428;
  inline static auto constexpr kTooManyRequests = 429;
  inline static auto constexpr kRequestHeaderFieldsTooLarge = 431;
  inline static auto constexpr kUnavailableForLegalReasons = 451;
  // Server error responses (5XX)
  inline static auto constexpr kInternalServerError = 500;
  inline static auto constexpr kNotImplemented = 501;
  inline static auto constexpr kBadGateway = 502;
  inline static auto constexpr kServiceUnavailable = 503;
  inline static auto constexpr kGatewayTimeout = 504;
  inline static auto constexpr kHTTPVersionNotSupported = 505;
  inline static auto constexpr kVariantAlsoNegotiates = 506;
  inline static auto constexpr kInsufficientStorage = 507;
  inline static auto constexpr kLoopDetected = 508;
  inline static auto constexpr kNotExtended = 510;
  inline static auto constexpr kNetworkAuthenticationRequired = 511;
  //@}

  class Impl {
   public:
    virtual ~Impl() = 0;
    virtual void set_payload(std::string) = 0;
    [[nodiscard]] virtual std::string const& payload() const = 0;
    virtual void set_result(int code) = 0;
    [[nodiscard]] virtual int result() const = 0;
    virtual void set_header(std::string_view name, std::string_view value) = 0;
    [[nodiscard]] virtual HeadersType headers() const = 0;
    virtual void set_version(int major, int minor) = 0;
    [[nodiscard]] virtual int version_major() const = 0;
    [[nodiscard]] virtual int version_minor() const = 0;
  };

  explicit HttpResponse(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}

 private:
  friend struct functions_internal::UnwrapResponse;
  std::shared_ptr<Impl> impl_;
};

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions

#endif  // FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_HTTP_RESPONSE_H
