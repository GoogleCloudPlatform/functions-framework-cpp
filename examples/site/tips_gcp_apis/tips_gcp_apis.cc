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

// [START functions_pubsub_publish]
// [START functions_tips_gcp_apis]
#include <google/cloud/functions/function.h>
#include <google/cloud/pubsub/blocking_publisher.h>
#include <nlohmann/json.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace gcf = ::google::cloud::functions;
namespace pubsub = ::google::cloud::pubsub;

namespace {
pubsub::BlockingPublisher GetPublisher() {
  static std::mutex mu;
  static std::shared_ptr<pubsub::BlockingPublisherConnection> connection;

  std::lock_guard const lk(mu);
  if (!connection) connection = pubsub::MakeBlockingPublisherConnection();
  return pubsub::BlockingPublisher(connection);
}
}  // namespace

gcf::HttpResponse tips_gcp_apis_impl(gcf::HttpRequest const& request) {
  auto const* project = std::getenv("GCP_PROJECT");
  if (project == nullptr) throw std::runtime_error("GCP_PROJECT is not set");

  auto const body = nlohmann::json::parse(request.payload());
  auto const topic_id = body.value("topic", "");
  if (topic_id.empty()) throw std::runtime_error("Missing topic in request");

  auto publisher = GetPublisher();
  auto id = publisher.Publish(
      pubsub::Topic(project, topic_id),
      pubsub::MessageBuilder().SetData("Test message").Build());

  if (!id) {
    return gcf::HttpResponse{}.set_result(
        gcf::HttpResponse::kInternalServerError);
  }
  return gcf::HttpResponse{}
      .set_payload("1 message published")
      .set_header("content-type", "text/plain");
}

gcf::Function tips_gcp_apis() { return gcf::MakeFunction(tips_gcp_apis_impl); }
// [END functions_tips_gcp_apis]
// [END functions_pubsub_publish]
