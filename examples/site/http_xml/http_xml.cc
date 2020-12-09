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
#include <google/cloud/functions/http_request.h>
#include <google/cloud/functions/http_response.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <sstream>

namespace gcf = ::google::cloud::functions;

gcf::HttpResponse http_xml(gcf::HttpRequest request) {  // NOLINT
  std::istringstream is(request.payload());
  // Use the Boost.PropertyTree XML parser, this is adequate for a small
  // example, but application developers may want to consider a more robust
  // parser for production code.
  boost::property_tree::ptree data;
  boost::property_tree::read_xml(is, data);

  auto name = data.get<std::string>("name", "World");
  gcf::HttpResponse response;
  response.set_header("content-type", "text/plain");
  response.set_payload("Hello " + name);
  return response;
}
// [END functions_http_xml]
