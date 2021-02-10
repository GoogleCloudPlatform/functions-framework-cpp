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
#include <google/cloud/functions/http_request.h>
#include <google/cloud/functions/http_response.h>
#include <google/cloud/pubsub/publisher.h>
#include <nlohmann/json.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace gcf = ::google::cloud::functions;
namespace pubsub = ::google::cloud::pubsub;

namespace {
pubsub::Publisher GetPublisher(pubsub::Topic topic) {
  using Map = std::unordered_map<std::string,
                                 std::shared_ptr<pubsub::PublisherConnection>>;

  static std::mutex mu;
  static Map connections;

  std::lock_guard<std::mutex> lk(mu);
  auto [pos, inserted] = connections.emplace(
      topic.FullName(), std::shared_ptr<pubsub::PublisherConnection>());
  if (inserted) {
    pos->second = pubsub::MakePublisherConnection(std::move(topic), {});
  }
  return pubsub::Publisher(pos->second);
}
}  // namespace

gcf::HttpResponse tips_gcp_apis(gcf::HttpRequest request) {  // NOLINT
  auto const* project = std::getenv("GCP_PROJECT");
  if (project == nullptr) throw std::runtime_error("GCP_PROJECT is not set");

  auto const body = nlohmann::json::parse(request.payload());
  auto const topic_id = body.value("topic", "");
  if (topic_id.empty()) throw std::runtime_error("Missing topic in request");

  auto publisher = GetPublisher(pubsub::Topic(project, topic_id));
  auto id = publisher.Publish(
      pubsub::MessageBuilder().SetData("Test message").Build());

  gcf::HttpResponse response;
  if (!id.get()) {
    response.set_result(gcf::HttpResponse::kInternalServerError);
  } else {
    response.set_payload("1 message published");
    response.set_header("content-type", "text/plain");
  }
  return response;
}
// [END functions_tips_gcp_apis]
// [END functions_pubsub_publish]
