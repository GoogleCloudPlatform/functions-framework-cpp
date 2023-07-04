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

// [START functions_http_form_data]
#include <google/cloud/functions/http_request.h>
#include <google/cloud/functions/http_response.h>
#include <absl/strings/str_split.h>
#include <absl/strings/string_view.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <functional>
#include <regex>
#include <vector>

namespace gcf = ::google::cloud::functions;

namespace {
class FormDataDelimiter {
 public:
#if defined(__APPLE__) && defined(__clang__)
  using searcher = std::default_searcher<absl::string_view::iterator>;
#else
  using searcher =
      std::boyer_moore_horspool_searcher<absl::string_view::iterator>;
#endif  // ! __APPLE__ && __clang__
  static FormDataDelimiter FromHeader(std::string const& header);
  [[nodiscard]] absl::string_view Find(absl::string_view text,
                                       std::size_t pos) const;

 private:
  explicit FormDataDelimiter(std::string body, std::string part)
      : body_marker_(std::move(body)),
        body_marker_view_(body_marker_),
        body_marker_searcher_(body_marker_view_.begin(),
                              body_marker_view_.end()),
        part_marker_(std::move(part)),
        part_marker_view_(part_marker_),
        part_marker_searcher_(part_marker_view_.begin(),
                              part_marker_view_.end()) {}

  std::string body_marker_;
  absl::string_view body_marker_view_;
  searcher body_marker_searcher_;
  std::string part_marker_;
  absl::string_view part_marker_view_;
  searcher part_marker_searcher_;
};

}  // namespace

gcf::HttpResponse http_form_data(gcf::HttpRequest request) {
  if (request.verb() != "POST") {
    return gcf::HttpResponse{}.set_result(gcf::HttpResponse::kMethodNotAllowed);
  }
  auto const header = request.headers().find("content-type");
  if (header == request.headers().end()) {
    return gcf::HttpResponse{}.set_result(gcf::HttpResponse::kBadRequest);
  }
  auto const& content_type = header->second;
  if (content_type.rfind("multipart/form-data", 0) != 0) {
    return gcf::HttpResponse{}.set_result(gcf::HttpResponse::kBadRequest);
  }
  auto delimiter = FormDataDelimiter::FromHeader(header->second);

  auto payload = std::move(request).payload();
  std::vector<absl::string_view> parts =
      absl::StrSplit(payload, delimiter, absl::SkipEmpty{});
  nlohmann::json result{{"parts", std::vector<nlohmann::json>{}}};
  for (auto& p : parts) {
    std::vector<absl::string_view> components =
        absl::StrSplit(p, absl::MaxSplits("\r\n\r\n", 2));
    auto const body_size = components.size() == 2 ? components[0].size() : 0;

    std::vector<std::string> const part_headers =
        absl::StrSplit(components[0], "\r\n");
    nlohmann::json descriptor{{"bodySize", body_size},
                              {"headerCount", part_headers.size()}};
    for (auto const& h : part_headers) {
      static auto const kContentDispositionRE = std::regex(
          R"re(^content-disposition:)re",
          std::regex::extended | std::regex::icase | std::regex::optimize);
      static auto const kNameRE =
          std::regex(R"re([;[:space:]]name=([^[:space:];]+))re",
                     std::regex::extended | std::regex::icase);
      // NOTE: this does not handle embedded spaces and/or semi-colons in the
      // filename, and so should be improved for production code.
      static auto const kFilenameRE =
          std::regex(R"re([;[:space:]]filename=([^[:space:];]+))re",
                     std::regex::extended | std::regex::icase);

      if (!std::regex_search(h.begin(), h.end(), kContentDispositionRE)) {
        continue;
      }
      std::smatch m;
      if (std::regex_search(h.begin(), h.end(), m, kNameRE)) {
        descriptor["name"] = m[1];
      }

      if (!std::regex_search(h.begin(), h.end(), m, kFilenameRE)) continue;
      descriptor["isFile"] = true;
      descriptor["filename"] = m[1];
    }
    result["parts"].push_back(std::move(descriptor));
  }

  return gcf::HttpResponse{}
      .set_header("content-type", "application/json")
      .set_payload(result.dump());
}
// [END functions_http_form_data]

namespace {
std::string GetBoundary(std::string const& header) {
  static auto const kBoundaryRE =
      std::regex(R"re([;[:space:]]boundary=([^;[:space:]]+))re",
                 std::regex::icase | std::regex::extended);
  std::smatch m;
  if (std::regex_search(header.begin(), header.end(), m, kBoundaryRE)) {
    auto match = std::string(m[1].first, m[1].second);
    if (match.back() == '"' && match.front() == '"') {
      return match.substr(1, match.size() - 2);
    }
    return match;
  }
  throw std::invalid_argument("cannot find 'boundary=' parameter in header=" +
                              std::string(header));
}

FormDataDelimiter FormDataDelimiter::FromHeader(std::string const& header) {
  auto b = GetBoundary(header);
  return FormDataDelimiter("\r\n--" + b + "--\r\n", "\r\n--" + b + "\r\n");
}

[[nodiscard]] absl::string_view FormDataDelimiter::Find(absl::string_view text,
                                                        std::size_t pos) const {
  auto const* found =
      std::search(text.begin() + pos, text.end(), part_marker_searcher_);
  if (found != text.end()) {
    return absl::string_view(found, part_marker_.size());
  }
  found = std::search(text.begin() + pos, text.end(), body_marker_searcher_);
  if (found != text.end()) {
    return absl::string_view(found, body_marker_.size());
  }
  return absl::string_view(text.data() + text.size(), 0);
}

}  // namespace
