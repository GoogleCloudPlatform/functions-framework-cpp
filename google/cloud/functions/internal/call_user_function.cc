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

#include "google/cloud/functions/internal/call_user_function.h"
#include "google/cloud/functions/internal/parse_cloud_event_http.h"
#include "google/cloud/functions/internal/wrap_request.h"
#include "google/cloud/functions/internal/wrap_response.h"
#include <iostream>
#include <stdexcept>

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {

namespace be = ::boost::beast;

struct UnwrapResponse {
  static BeastResponse unwrap(functions::HttpResponse response) {
    auto impl = std::move(response).impl_;
    auto& wrap = dynamic_cast<WrapResponseImpl&>(*impl);
    return std::move(wrap).response_;
  }
};

BeastResponse CallUserFunction(functions::UserHttpFunction const& function,
                               BeastRequest request) try {
  if (request.target() == "/favicon.ico" || request.target() == "/robots.txt") {
    BeastResponse response;
    response.result(be::http::status::not_found);
    return response;
  }
  auto response = function(MakeHttpRequest(std::move(request)));
  return UnwrapResponse::unwrap(std::move(response));
} catch (std::exception const& ex) {
  std::cerr << "standard C++ exception thrown: " << ex.what() << std::endl;
  BeastResponse response;
  response.result(be::http::status::internal_server_error);
  return response;
} catch (...) {
  std::cerr << "unknown c++ exception thrown" << std::endl;
  BeastResponse response;
  response.result(be::http::status::internal_server_error);
  return response;
}

BeastResponse CallUserFunction(
    functions::UserCloudEventFunction const& function,
    BeastRequest const& request) try {
  if (request.target() == "/favicon.ico" || request.target() == "/robots.txt") {
    BeastResponse response;
    response.result(be::http::status::not_found);
    return response;
  }
  auto const events = ParseCloudEventHttp(request);
  for (auto const& ce : events) {
    function(ce);
  }
  return BeastResponse{};
} catch (std::exception const& ex) {
  std::cerr << "standard C++ exception thrown: " << ex.what() << std::endl;
  BeastResponse response;
  response.result(be::http::status::internal_server_error);
  return response;
} catch (...) {
  std::cerr << "unknown c++ exception thrown" << std::endl;
  BeastResponse response;
  response.result(be::http::status::internal_server_error);
  return response;
}

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal
