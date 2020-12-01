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

#include "google/cloud/functions/internal/parse_cloud_event_json.h"
#include <gmock/gmock.h>

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {
namespace {

TEST(ParseCloudEventJson, Basic) {
  auto constexpr kText = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234"})js";
  auto ce = ParseCloudEventJson(kText);
  EXPECT_EQ(ce.id(), "A234-1234-1234");
  EXPECT_EQ(ce.source(), "/mycontext");
  EXPECT_EQ(ce.type(), "com.example.someevent");
  EXPECT_EQ(ce.spec_version(), functions::CloudEvent::kDefaultSpecVersion);

  EXPECT_THROW(ParseCloudEventJson("{"), std::exception);
}

TEST(ParseCloudEventJson, WithSpecVersion) {
  auto constexpr kText = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "specversion" : "1.1"})js";
  auto ce = ParseCloudEventJson(kText);
  EXPECT_EQ(ce.id(), "A234-1234-1234");
  EXPECT_EQ(ce.source(), "/mycontext");
  EXPECT_EQ(ce.type(), "com.example.someevent");
  EXPECT_EQ(ce.spec_version(), "1.1");
}

TEST(ParseCloudEventJson, MissingRequiredField) {
  auto constexpr kMissingId = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext"})js";
  auto constexpr kMissingType = R"js({
    "source" : "/mycontext",
    "id" : "A234-1234-1234"})js";
  auto constexpr kMissingSource = R"js({
    "type" : "com.example.someevent",
    "id" : "A234-1234-1234"})js";
  EXPECT_THROW(ParseCloudEventJson(kMissingId), std::exception);
  EXPECT_THROW(ParseCloudEventJson(kMissingType), std::exception);
  EXPECT_THROW(ParseCloudEventJson(kMissingSource), std::exception);
}

TEST(ParseCloudEventJson, InvalidRequiredField) {
  auto constexpr kInvalidId = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : [ "A234-1234-1234" ]})js";
  auto constexpr kInvalidType = R"js({
    "type" : [ "com.example.someevent" ],
    "source" : "/mycontext",
    "id" : "A234-1234-1234"})js";
  auto constexpr kInvalidSource = R"js({
    "type" : "com.example.someevent",
    "source" : [ "/mycontext" ],
    "id" : "A234-1234-1234"})js";
  EXPECT_THROW(ParseCloudEventJson(kInvalidId), std::exception);
  EXPECT_THROW(ParseCloudEventJson(kInvalidType), std::exception);
  EXPECT_THROW(ParseCloudEventJson(kInvalidSource), std::exception);
}

TEST(ParseCloudEventJson, WithDataContentType) {
  auto constexpr kText = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "datacontenttype" : "application/json"})js";
  auto constexpr kTextInvalid = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "datacontenttype" : {"foo": "bar"}})js";
  auto const ce = ParseCloudEventJson(kText);
  EXPECT_THAT(ce.data_content_type().value_or(""), "application/json");
  EXPECT_THROW(ParseCloudEventJson(kTextInvalid), std::exception);
}

TEST(ParseCloudEventJson, WithDataSchema) {
  auto constexpr kText = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "dataschema" : "test-schema"})js";
  auto constexpr kTextInvalid = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "dataschema" : {"foo": "bar"}})js";
  auto const ce = ParseCloudEventJson(kText);
  EXPECT_THAT(ce.data_schema().value_or(""), "test-schema");
  EXPECT_THROW(ParseCloudEventJson(kTextInvalid), std::exception);
}

TEST(ParseCloudEventJson, WithSubject) {
  auto constexpr kText = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "subject" : "test-subject"})js";
  auto constexpr kTextInvalid = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "subject" : {"foo": "bar"}})js";
  auto const ce = ParseCloudEventJson(kText);
  EXPECT_THAT(ce.subject().value_or(""), "test-subject");
  EXPECT_THROW(ParseCloudEventJson(kTextInvalid), std::exception);
}

TEST(ParseCloudEventJson, WithTime) {
  auto constexpr kText = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "time" : "2018-04-05T17:31:05Z"})js";
  auto constexpr kTextInvalid = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "time" : {"foo": "bar"}})js";
  // obtained using: date -u --date='2018-04-05T17:31:05Z' +%s
  auto constexpr kExpectedTime = std::chrono::seconds(1522949465L);
  auto const ce = ParseCloudEventJson(kText);
  auto tp = ce.time().value_or(std::chrono::system_clock::from_time_t(0));
  // This assumes the platform uses a unix epoch, reasonably safe assumption for
  // our needs.
  EXPECT_EQ(tp, std::chrono::system_clock::from_time_t(kExpectedTime.count()));
  EXPECT_THROW(ParseCloudEventJson(kTextInvalid), std::exception);
}

TEST(ParseCloudEventJson, WithData) {
  auto constexpr kText = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "data" : "some text"})js";
  auto constexpr kTextInvalid = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "dataschema" : {"foo": "bar"}})js";
  auto const ce = ParseCloudEventJson(kText);
  EXPECT_EQ(ce.data().value_or(""), "some text");
  EXPECT_THROW(ParseCloudEventJson(kTextInvalid), std::exception);
}

TEST(ParseCloudEventJson, WithDataBase64) {
  // Obtained magic string using:
  //   echo "some text" | openssl base64 -e
  auto constexpr kText = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "data_base64" : "c29tZSB0ZXh0Cg=="})js";
  auto constexpr kTextInvalid = R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "dataschema" : {"foo": "bar"}})js";
  auto const ce = ParseCloudEventJson(kText);
  EXPECT_EQ(ce.data().value_or(""), "some text\n");
  EXPECT_THROW(ParseCloudEventJson(kTextInvalid), std::exception);
}

TEST(ParseCloudEventJson, WithDataBase64Padding) {
  // Obtained magic string using:
  //   echo -n "" | openssl base64 -e
  //   echo -n "a" | openssl base64 -e
  //   echo -n "ab" | openssl base64 -e
  //   echo -n "abc" | openssl base64 -e
  struct Test {
    std::string expected_data;
    std::string json;
  } cases[] = {
      {"", R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "data_base64" : ""})js"},
      {"a", R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "data_base64" : "YQ=="})js"},
      {"ab", R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "data_base64" : "YWI="})js"},
      {"abc", R"js({
    "type" : "com.example.someevent",
    "source" : "/mycontext",
    "id" : "A234-1234-1234",
    "data_base64" : "YWJj"})js"},
  };
  for (auto const& c : cases) {
    auto const ce = ParseCloudEventJson(c.json);
    EXPECT_EQ(ce.data().value_or(""), c.expected_data);
  }
}

}  // namespace
}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal
