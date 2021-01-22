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

// [START functions_pubsub_unit_test]
#include <google/cloud/functions/cloud_event.h>
#include <boost/log/core.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/shared_ptr.hpp>
#include <gmock/gmock.h>
#include <nlohmann/json.hpp>
#include <memory>
#include <sstream>

namespace gcf = ::google::cloud::functions;
extern void hello_world_pubsub(gcf::CloudEvent event);

namespace {

using ::testing::HasSubstr;

TEST(PubsubUnitTest, Basic) {
  auto core = boost::log::core::get();
  auto stream = boost::make_shared<std::ostringstream>();
  auto be = [core, stream]() {
    auto backend =
        boost::make_shared<boost::log::sinks::text_ostream_backend>();
    backend->add_stream(stream);

    // Enable auto-flushing after each log record written
    backend->auto_flush(true);
    using sink_t = boost::log::sinks::synchronous_sink<
        boost::log::sinks::text_ostream_backend>;
    auto be = boost::make_shared<sink_t>(backend);
    core->add_sink(be);
    return be;
  }();

  struct TestCases {
    std::string data;
    std::string expected;
  } cases[]{
      // The magic string was obtained using:
      //  /bin/echo -n 'C++' | openssl base64 -e
      {R"js({"message": {"data": "Qysr"}})js", "Hello C++"},
      {R"js({"message": {"data": ""}})js", "Hello World"},
  };

  for (auto const& test : cases) {
    SCOPED_TRACE("Testing for " + test.expected);
    gcf::CloudEvent event(
        /*id=*/"test-id-0001", /*source=*/"https://test-source.example.com",
        /*type=*/"google.cloud.pubsub.topic.v1.messagePublished");
    event.set_data_content_type("application/json");
    event.set_data(test.data);
    stream->str({});
    EXPECT_NO_THROW(hello_world_pubsub(event));
    auto log_lines = stream->str();
    EXPECT_THAT(log_lines, HasSubstr(test.expected));
  }

  core->remove_sink(be);
}

}  // namespace
// [END functions_pubsub_unit_test]
