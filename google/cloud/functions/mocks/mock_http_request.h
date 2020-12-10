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

#ifndef FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_MOCKS_MOCK_HTTP_REQUEST_H
#define FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_MOCKS_MOCK_HTTP_REQUEST_H

#include "google/cloud/functions/http_request.h"
#include "google/cloud/functions/version.h"
#include <gmock/gmock.h>

namespace google::cloud::functions_mocks {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {

class MockHttpRequest : public google::cloud::functions::HttpRequest::Impl {
 public:
  MOCK_METHOD(std::string const&, verb, (), (const override));
  MOCK_METHOD(std::string const&, target, (), (const override));
  MOCK_METHOD(std::string const&, payload, (), (const override));
  MOCK_METHOD(google::cloud::functions::HttpRequest::HeadersType const&,
              headers, (), (const override));
  MOCK_METHOD(int, version_major, (), (const override));
  MOCK_METHOD(int, version_minor, (), (const override));
};

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_mocks

#endif  // FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_MOCKS_MOCK_HTTP_REQUEST_H
