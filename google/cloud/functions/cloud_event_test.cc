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

#include "google/cloud/functions/cloud_event.h"
#include <absl/time/time.h>
#include <gmock/gmock.h>

namespace google::cloud::functions {
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_BEGIN
namespace {

TEST(CloudEventTest, Basic) {
  auto actual = CloudEvent("test-id", "test-source", "test-type");
  EXPECT_EQ(actual.id(), "test-id");
  EXPECT_EQ(actual.source(), "test-source");
  EXPECT_EQ(actual.type(), "test-type");
  EXPECT_EQ(actual.spec_version(), CloudEvent::kDefaultSpecVersion);

  EXPECT_FALSE(actual.data_content_type().has_value());
  EXPECT_FALSE(actual.data_schema().has_value());
  EXPECT_FALSE(actual.subject().has_value());
  EXPECT_FALSE(actual.time().has_value());
  EXPECT_FALSE(actual.data().has_value());
  EXPECT_FALSE(std::move(actual).data().has_value());
}

TEST(CloudEventTest, WithVersion) {
  auto const actual = CloudEvent("test-id", "test-source", "test-type", "1.1");
  EXPECT_EQ(actual.spec_version(), "1.1");
}

TEST(CloudEventTest, DataContentType) {
  auto actual = CloudEvent("test-id", "test-source", "test-type");
  actual.set_data_content_type("test-value");
  EXPECT_EQ(actual.data_content_type().value_or(""), "test-value");
  actual.reset_data_content_type();
  EXPECT_FALSE(actual.data_content_type().has_value());
}

TEST(CloudEventTest, DataSchema) {
  auto actual = CloudEvent("test-id", "test-source", "test-type");
  actual.set_data_schema("test-value");
  EXPECT_EQ(actual.data_schema().value_or(""), "test-value");
  actual.reset_data_schema();
  EXPECT_FALSE(actual.data_schema().has_value());
}

TEST(CloudEventTest, Subject) {
  auto actual = CloudEvent("test-id", "test-source", "test-type");
  actual.set_subject("test-value");
  EXPECT_EQ(actual.subject().value_or(""), "test-value");
  actual.reset_subject();
  EXPECT_FALSE(actual.subject().has_value());
}

TEST(CloudEventTest, Time) {
  auto actual = CloudEvent("test-id", "test-source", "test-type");
  auto const tp = CloudEvent::ClockType::now();
  actual.set_time(tp);
  EXPECT_EQ(actual.time().value_or(tp + std::chrono::seconds(5)), tp);
  actual.reset_time();
  EXPECT_FALSE(actual.time().has_value());
}

TEST(CloudEventTest, TimeString) {
  auto actual = CloudEvent("test-id", "test-source", "test-type");
  std::string const valid[] = {
      "2020-11-30T12:34:45Z", "2020-11-30T12:34:45.678Z",
      "2020-11-30T12:34:45.678-05:00", "2020-11-30T12:34:45.678+05:00"};
  auto to_system_clock_tp = [](std::string const& s) {
    std::string err;
    absl::Time t;
    EXPECT_TRUE(absl::ParseTime(absl::RFC3339_full, s, &t, &err)) << s;
    return absl::ToChronoTime(t);
  };
  for (auto const& s : valid) {
    auto const tp = to_system_clock_tp(s);
    actual.set_time(s);
    ASSERT_TRUE(actual.time().has_value());
    EXPECT_EQ(*actual.time(), tp);
  }
  EXPECT_THROW(actual.set_time(""), std::exception);
  EXPECT_THROW(actual.set_time("2020-15"), std::exception);
}

TEST(CloudEventTest, Data) {
  auto actual = CloudEvent("test-id", "test-source", "test-type");
  actual.set_data("test-value");
  EXPECT_EQ(actual.data().value_or(""), "test-value");
  actual.reset_data();
  EXPECT_FALSE(actual.data().has_value());
}

TEST(CloudEventTest, DataMove) {
  auto actual = CloudEvent("test-id", "test-source", "test-type");
  actual.set_data("test-value");
  auto d = std::move(actual).data();
  EXPECT_EQ(d.value_or(""), "test-value");
}

}  // namespace
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_END
}  // namespace google::cloud::functions
