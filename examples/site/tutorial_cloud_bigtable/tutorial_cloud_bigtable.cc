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

// [START bigtable_functions_quickstart]
#include <google/cloud/bigtable/table.h>
#include <google/cloud/functions/http_request.h>
#include <google/cloud/functions/http_response.h>
#include <algorithm>
#include <cctype>
#include <mutex>
#include <sstream>
#include <string>

namespace gcf = ::google::cloud::functions;
namespace cbt = ::google::cloud::bigtable;

cbt::Table get_table_client(std::string project_id, std::string instance_id,
                            std::string const& table_id) {
  static std::mutex mu;
  static std::unique_ptr<cbt::Table> table;
  std::lock_guard lk(mu);
  if (table == nullptr || table->table_id() != table_id ||
      table->instance_id() != instance_id ||
      table->project_id() != project_id) {
    table = std::make_unique<cbt::Table>(
        cbt::CreateDefaultDataClient(std::move(project_id),
                                     std::move(instance_id),
                                     cbt::ClientOptions{}),
        table_id);
  }
  return *table;
}

gcf::HttpResponse tutorial_cloud_bigtable(gcf::HttpRequest request) {  // NOLINT
  auto get_header = [h = request.headers()](std::string key) {
    std::transform(key.begin(), key.end(), key.begin(),
                   [](auto x) { return static_cast<char>(std::tolower(x)); });
    auto l = h.find(key);
    if (l == h.end()) throw std::invalid_argument("Missing header: " + key);
    return l->second;
  };

  auto project_id = get_header("projectID");
  auto instance_id = get_header("instanceID");
  auto table_id = get_header("tableID");
  auto table =
      get_table_client(std::move(project_id), std::move(instance_id), table_id);

  std::ostringstream os;
  auto filter =
      cbt::Filter::Chain(cbt::Filter::Latest(1),
                         cbt::Filter::ColumnName("stats_summary", "os_build"));
  for (auto row : table.ReadRows(cbt::RowRange::Prefix("phone#"), filter)) {
    if (!row) {
      std::ostringstream err;
      err << "Error reading from bigtable: " << row.status();
      throw std::runtime_error(std::move(err).str());
    }
    for (auto const& cell : row->cells()) {
      os << "Rowkey: " << cell.row_key() << ", os_build: " << cell.value()
         << "\n";
    }
  }

  return gcf::HttpResponse{}
      .set_payload(std::move(os).str())
      .set_header("content-type", "text/plain");
}
// [END bigtable_functions_quickstart]
