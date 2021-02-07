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

// [START functions_helloworld_pubsub]
#include <google/cloud/functions/cloud_event.h>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/log/trivial.hpp>
#include <nlohmann/json.hpp>
#include <iostream>

namespace gcf = ::google::cloud::functions;

// Use Boost.Archive to decode Pub/Sub message payload
std::string decode_base64(std::string const& base64);

void hello_world_pubsub(gcf::CloudEvent event) {  // NOLINT
  if (event.data_content_type().value_or("") != "application/json") {
    std::cerr << "Error: expected application/json data\n";
    return;
  }
  auto const payload = nlohmann::json::parse(event.data().value_or("{}"));
  auto name = decode_base64(payload["message"]["data"].get<std::string>());
  BOOST_LOG_TRIVIAL(info) << "Hello " << (name.empty() ? "World" : name);
}
// [END functions_helloworld_pubsub]

std::string decode_base64(std::string const& base64) {
  namespace bai = boost::archive::iterators;
  auto constexpr kBase64RawBits = 6;
  auto constexpr kBase64EncodedBits = 8;
  using Decoder =
      bai::transform_width<bai::binary_from_base64<std::string::iterator>,
                           kBase64EncodedBits, kBase64RawBits>;
  // Pad the raw string if needed.
  auto padded = base64;
  padded.append((4 - padded.size() % 4) % 4, '=');
  // While we know how much padding we added, there may have been some padding
  // there, just not enough. We need to determine the actual number of `=`
  // characters at the end of the string.
  auto pad_count = std::distance(padded.rbegin(),
                                 std::find_if(padded.rbegin(), padded.rend(),
                                              [](auto c) { return c != '='; }));
  if (pad_count > 2) {
    throw std::invalid_argument("Invalid base64 string <" + base64 + ">");
  }

  std::string data{Decoder(padded.begin()), Decoder(padded.end())};
  for (; pad_count != 0; --pad_count) data.pop_back();
  return data;
}
