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

#include "google/cloud/functions/internal/function_impl.h"
#include "google/cloud/functions/function.h"
#include <gmock/gmock.h>

namespace gcf = ::google::cloud::functions;
namespace gcf_internal = ::google::cloud::functions_internal;

gcf::Function HelloCloudEvent();

namespace {

auto constexpr kBody = R"js({
  "specversion": "1.0",
  "type": "google.cloud.pubsub.topic.v1.messagePublished",
  "source": "//pubsub.googleapis.com/projects/sample-project/topics/gcf-test",
  "id": "aaaaaa-1111-bbbb-2222-cccccccccccc",
  "subject": "test-only",
  "time": "2020-09-29T11:32:00.000Z",
  "datacontenttype": "application/json",
  "data": {
    "subscription": "projects/sample-project/subscriptions/sample-subscription",
    "message": {
      "@type": "type.googleapis.com/google.pubsub.v1.PubsubMessage",
      "attributes": {
         "attr1":"attr1-value"
      },
      "data": ""
    }
  }
})js";

TEST(HttpExamplesTest, HelloCloudEvent) {
  auto function = HelloCloudEvent();
  gcf_internal::BeastRequest request;
  request.insert("ce-type", "com.example.someevent");
  request.insert("ce-source", "/mycontext");
  request.insert("ce-id", "A234-1234-1234");
  request.body() = kBody;
  auto handler =
      gcf_internal::FunctionImpl::GetImpl(function)->GetHandler("unused");
  auto const response = handler(std::move(request));
  EXPECT_EQ(response.result_int(), 200);
}

}  // namespace
