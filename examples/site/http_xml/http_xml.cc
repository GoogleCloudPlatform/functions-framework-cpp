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

// [START functions_http_xml]
#include <google/cloud/functions/function.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <sstream>

namespace gcf = ::google::cloud::functions;

gcf::Function http_xml() {
  return gcf::MakeFunction([](gcf::HttpRequest const& request) {
    std::istringstream is(request.payload());
    // Use the Boost.PropertyTree XML parser, as this is adequate for a small
    // example. Application developers may want to consider a more robust
    // parser for production code.
    boost::property_tree::ptree data;
    boost::property_tree::read_xml(is, data);

    auto name = data.get<std::string>("name", "World");
    return gcf::HttpResponse{}
        .set_header("content-type", "text/plain")
        .set_payload("Hello " + name);
  });
}
// [END functions_http_xml]
