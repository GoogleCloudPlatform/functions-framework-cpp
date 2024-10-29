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

// [START functions_pubsub_system_test]
#include <google/cloud/pubsub/admin/topic_admin_client.h>
#include <google/cloud/pubsub/publisher.h>
#include <boost/process.hpp>
#include <fmt/format.h>
#include <gmock/gmock.h>
#include <chrono>
#include <string>
#include <thread>

namespace {

namespace bp = ::boost::process;
namespace pubsub = ::google::cloud::pubsub;
namespace pubsub_admin = ::google::cloud::pubsub_admin;
using ::testing::AnyOf;
using ::testing::HasSubstr;

class PubsubSystemTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // This test can only run if it is properly configured.
    auto const* project_id = std::getenv("GOOGLE_CLOUD_PROJECT");
    ASSERT_NE(project_id, nullptr);
    auto const* topic_id = std::getenv("TOPIC_ID");
    ASSERT_NE(topic_id, nullptr);
    auto const* service_id = std::getenv("SERVICE_ID");
    ASSERT_NE(service_id, nullptr);
    service_id_ = service_id;

    // Automatically setup the test environment, create the topic if it does
    // not exist.
    auto admin = pubsub_admin::TopicAdminClient(
        pubsub_admin::MakeTopicAdminConnection());
    topic_ = pubsub::Topic(project_id, topic_id);
    auto topic_metadata = admin.CreateTopic(topic_.FullName());
    // If we get an error other than kAlreadyExists, abort the test.
    ASSERT_THAT(topic_metadata.status().code(),
                AnyOf(google::cloud::StatusCode::kOk,
                      google::cloud::StatusCode::kAlreadyExists));
  }

  void TearDown() override {}

  [[nodiscard]] pubsub::Topic const& topic() const { return topic_; }
  [[nodiscard]] std::string const& service_id() const { return service_id_; }

 private:
  pubsub::Topic topic_ = pubsub::Topic("--invalid--", "--invalid--");
  std::string service_id_;
};

TEST_F(PubsubSystemTest, Basic) {
  struct {
    std::string data;
    std::string expected;
  } const cases[]{
      {"C++", "Hello C++"},
      {"World", "Hello World"},
  };

  auto publisher = pubsub::Publisher(pubsub::MakePublisherConnection(topic()));

  for (auto const& test : cases) {
    SCOPED_TRACE("Testing for " + test.expected);
    auto id =
        publisher.Publish(pubsub::MessageBuilder().SetData(test.data).Build());
    publisher.Flush();
    EXPECT_TRUE(id.get().ok());

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
                              fmt::format(kProject, topic().project_id()),
                              fmt::format(kLogFilter, service_id()),
                              "--format=value(textPayload)", "--limit=1",
                              (bp::std_out & bp::std_err) > out);
      reader.wait();
      ASSERT_EQ(reader.exit_code(), 0);
      for (std::string l; std::getline(out, l);) lines.push_back(std::move(l));
      auto count = std::count_if(lines.begin(), lines.end(),
                                 [s = test.expected](auto l) {
                                   return l.find(s) != std::string::npos;
                                 });
      if (count != 0) break;
      std::this_thread::sleep_for(delay);
    }
    EXPECT_THAT(lines, Contains(HasSubstr(test.expected)));
  }
}

}  // namespace

// [END functions_pubsub_system_test]
