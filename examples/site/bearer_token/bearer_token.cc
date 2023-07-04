// Copyright 2021 Google LLC
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

// [START functions_bearer_token]
#include <google/cloud/functions/function.h>
#include <google/cloud/storage/oauth2/google_credentials.h>
#include <curl/curl.h>
#include <cstdlib>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

namespace gcf = ::google::cloud::functions;

namespace {
/// A helper function to perform an HTTP GET request.
gcf::HttpResponse HttpGet(std::string const& url,
                          std::string const& authorization_header);
}  // namespace

gcf::HttpResponse bearer_token_impl(gcf::HttpRequest const& request) {
  static auto const kTargetUrl = [] {
    auto const* target_url = std::getenv("TARGET_URL");
    if (target_url != nullptr) return std::string(target_url);
    throw std::runtime_error("TARGET_URL environment variable is not defined");
  }();

  static auto const kCredentials = [] {
    auto credentials =
        google::cloud::storage::oauth2::GoogleDefaultCredentials();
    if (credentials) return std::move(credentials).value();
    std::ostringstream os;
    os << "Error fetching credentials: " << std::move(credentials).status();
    throw std::runtime_error(std::move(os).str());
  }();

  static auto const kCurlInit = [] {
    return curl_global_init(CURL_GLOBAL_ALL);
  }();
  static_cast<void>(kCurlInit);

  if (request.target() == "/no-auth-header") return HttpGet(kTargetUrl, "");

  auto header = kCredentials->AuthorizationHeader();
  if (header) return HttpGet(kTargetUrl, *header);

  std::ostringstream os;
  os << "Error creating authorization header: " << std::move(header).status();
  throw std::runtime_error(std::move(os).str());
}

gcf::Function bearer_token() { return gcf::MakeFunction(bearer_token_impl); }
// [END functions_bearer_token]

namespace {
extern "C" size_t CurlOnWriteData(char* ptr, size_t size, size_t nmemb,
                                  void* userdata) {
  auto* buffer = reinterpret_cast<std::string*>(userdata);
  buffer->append(ptr, size * nmemb);
  return size * nmemb;
}

gcf::HttpResponse HttpGet(std::string const& url,
                          std::string const& authorization_header) {
  using CurlHeaders =
      std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)>;
  auto headers = CurlHeaders(nullptr, curl_slist_free_all);
  auto add_header = [&headers](std::string const& h) {
    auto* nh = curl_slist_append(headers.get(), h.c_str());
    (void)headers.release();
    headers.reset(nh);
  };

  if (!authorization_header.empty()) add_header(authorization_header);

  using CurlHandle = std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>;
  auto easy = CurlHandle(curl_easy_init(), curl_easy_cleanup);

  auto setopt = [h = easy.get()](auto opt, auto value) {
    if (auto e = curl_easy_setopt(h, opt, value); e != CURLE_OK) {
      std::ostringstream os;
      os << "error [" << e << "] setting curl_easy option <" << opt
         << ">=" << value;
      throw std::runtime_error(std::move(os).str());
    }
  };
  auto get_response_code = [h = easy.get()]() {
    long code;  // NOLINT(google-runtime-int)
    auto e = curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &code);
    if (e == CURLE_OK) {
      return static_cast<int>(code);
    }
    throw std::runtime_error("Cannot get response code");
  };

  setopt(CURLOPT_URL, url.c_str());
  setopt(CURLOPT_WRITEFUNCTION, &CurlOnWriteData);
  std::string buffer;
  setopt(CURLOPT_WRITEDATA, &buffer);
  setopt(CURLOPT_HTTPHEADER, headers.get());

  auto e = curl_easy_perform(easy.get());
  gcf::HttpResponse response;
  if (e == CURLE_OK) {
    response.set_result(get_response_code());
    response.set_payload(std::move(buffer));
  } else {
    response.set_result(gcf::HttpResponse::kInternalServerError);
  }
  return response;
}

}  // namespace
