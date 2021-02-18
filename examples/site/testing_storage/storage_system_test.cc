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

// [START functions_storage_system_test]
#include <google/cloud/storage/client.h>
#include <boost/process.hpp>
#include <fmt/format.h>
#include <gmock/gmock.h>
#include <chrono>
#include <random>
#include <string>
#include <thread>

namespace {

namespace bp = ::boost::process;
namespace gcs = ::google::cloud::storage;
using ::testing::HasSubstr;

class StorageSystemTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // This test can only run if it is properly configured.
    auto const* project_id = std::getenv("GOOGLE_CLOUD_PROJECT");
    ASSERT_NE(project_id, nullptr);
    project_id_ = project_id;

    auto const* bucket_name = std::getenv("BUCKET_NAME");
    ASSERT_NE(bucket_name, nullptr);
    bucket_name_ = bucket_name;

    auto const* service_id = std::getenv("SERVICE_ID");
    ASSERT_NE(service_id, nullptr);
    service_id_ = service_id;
  }

  void TearDown() override {}

  [[nodiscard]] std::string const& project_id() const { return project_id_; }
  [[nodiscard]] std::string const& bucket_name() const { return bucket_name_; }
  [[nodiscard]] std::string const& service_id() const { return service_id_; }

 private:
  std::string project_id_;
  std::string bucket_name_;
  std::string service_id_;
};

TEST_F(StorageSystemTest, Basic) {
  auto client = gcs::Client::CreateDefaultClient();
  ASSERT_TRUE(client.status().ok());

  // Use a random name to avoid interference from other tests.
  auto gen = std::mt19937_64(std::random_device{}());
  auto rnd = [&gen] {
    return std::to_string(std::uniform_int_distribution<std::uint64_t>{}(gen));
  };
  auto const object_name = "test-" + rnd() + '-' + rnd() + '-' + rnd();
  auto const expected = "Object: " + object_name;
  SCOPED_TRACE("Testing for " + object_name);
  auto meta =
      client->InsertObject(bucket_name(), object_name, "Lorem ipsum...");
  ASSERT_TRUE(meta.status().ok());

  std::vector<std::string> lines;
  // It may take a few seconds for the logs to propagate, so try a few times.
  using namespace std::chrono_literals;
  for (auto delay : {1s, 2s, 4s, 8s, 16s}) {
    bp::ipstream out;
    auto constexpr kProject = "--project={}";
    auto constexpr kLogFilter =
        "resource.type=cloud_run_revision AND "
        "resource.labels.service_name={} AND "
        "logName:stdout";
    auto reader = bp::child(bp::search_path("gcloud"), "logging", "read",
                            fmt::format(kProject, project_id()),
                            fmt::format(kLogFilter, service_id()),
                            "--format=value(textPayload)", "--limit=100",
                            (bp::std_out & bp::std_err) > out);
    reader.wait();
    ASSERT_EQ(reader.exit_code(), 0);
    for (std::string l; std::getline(out, l);) lines.push_back(std::move(l));
    auto count = std::count_if(
        lines.begin(), lines.end(),
        [s = expected](auto l) { return l.find(s) != std::string::npos; });
    if (count != 0) break;
    std::this_thread::sleep_for(delay);
  }
  EXPECT_THAT(lines, Contains(HasSubstr(expected)));
  EXPECT_TRUE(client->DeleteObject(bucket_name(), object_name).ok());
}

}  // namespace

// [END functions_storage_system_test]
