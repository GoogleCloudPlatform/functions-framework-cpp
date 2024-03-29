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

// [START spanner_functions_quickstart]
#include <google/cloud/functions/function.h>
#include <google/cloud/spanner/client.h>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>

namespace gcf = ::google::cloud::functions;
namespace spanner = ::google::cloud::spanner;

namespace {

auto get_spanner_database() {
  auto getenv = [](char const* var) {
    auto const* value = std::getenv(var);
    if (value == nullptr) {
      throw std::runtime_error("Environment variable " + std::string(var) +
                               " is not set");
    }
    return std::string(value);
  };
  return spanner::Database(getenv("GOOGLE_CLOUD_PROJECT"),
                           getenv("SPANNER_INSTANCE"),
                           getenv("SPANNER_DATABASE"));
}

gcf::HttpResponse handle_request(gcf::HttpRequest const& /*request*/) {
  static auto const kClient =
      spanner::Client(spanner::MakeConnection(get_spanner_database()));

  auto client = kClient;

  std::ostringstream os;
  os << "Albums:\n";
  auto rows = client.ExecuteQuery(spanner::SqlStatement(
      "SELECT SingerId, AlbumId, AlbumTitle FROM Albums"));
  using RowType = std::tuple<std::int64_t, std::int64_t, std::string>;
  for (auto const& row : spanner::StreamOf<RowType>(rows)) {
    if (!row) throw std::runtime_error(row.status().message());
    os << std::get<0>(*row) << " " << std::get<1>(*row) << " "
       << std::get<2>(*row) << "\n";
  }

  return gcf::HttpResponse{}
      .set_header("content-type", "text/plain")
      .set_payload(std::move(os).str());
}

}  // namespace

gcf::Function tutorial_cloud_spanner() {
  return gcf::MakeFunction(handle_request);
}

// [END spanner_functions_quickstart]
