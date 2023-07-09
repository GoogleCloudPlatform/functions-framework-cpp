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

// [START functions_http_content]
#include <google/cloud/functions/function.h>
#include <nlohmann/json.hpp>
#include <charconv>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace gcf = ::google::cloud::functions;

namespace {
std::map<std::string, std::string> parse_www_form_urlencoded(
    std::string const& text);
}  // namespace

gcf::Function http_content() {
  return gcf::MakeFunction([](gcf::HttpRequest const& request) {
    std::string name;
    auto const& headers = request.headers();
    if (auto f = headers.find("content-type"); f != headers.end()) {
      if (f->second == "application/json") {
        name = nlohmann::json::parse(request.payload()).value("name", "");
      } else if (f->second == "application/octet-stream" ||
                 f->second == "text/plain") {
        name = request.payload();  // treat contents as a string
      } else if (f->second == "application/x-www-form-urlencoded") {
        // Use your preferred parser, here we use some custom code.
        auto form = parse_www_form_urlencoded(request.payload());
        name = form["name"];
      }
    }

    return gcf::HttpResponse{}
        .set_header("content-type", "text/plain")
        .set_payload("Hello " + name);
  });
}
// [END functions_http_content]

namespace {
std::string urldecode(std::string const& encoded) {
  std::string result;
  for (std::size_t i = 0; i != encoded.size(); ++i) {
    if (encoded[i] != '%') {
      result.push_back(encoded[i]);
      continue;
    }
    if (i + 3 <= encoded.size()) {
      auto constexpr kUrlEncodingBase = 16;
      char value;
      auto const* end = encoded.data() + i + 3;
      auto r =
          std::from_chars(encoded.data() + i + 1, end, value, kUrlEncodingBase);
      if (r.ec == std::errc{} && r.ptr == end) {
        result.push_back(value);
        i += 2;
      } else {
        result.push_back(encoded[i]);
      }
    }
  }
  return result;
}

/// Implements a parser for www-form-urlencoded data, a bit tedious, maybe not
/// the most efficient, but has very minimal dependencies and it is easy to
/// read.
std::map<std::string, std::string> parse_www_form_urlencoded(
    std::string const& text) {
  auto tokens = [&text] {
    std::vector<std::string> tokens;
    std::istringstream is(text);
    for (std::string tk; std::getline(is, tk, '&'); tokens.push_back(tk)) {
    }
    return tokens;
  }();
  std::map<std::string, std::string> result;
  for (auto& tk : tokens) {
    auto p = tk.find_first_of('=');
    result.emplace(urldecode(tk.substr(0, p)), urldecode(tk.substr(p + 1)));
  }
  return result;
}

}  // namespace
