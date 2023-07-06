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

#include "google/cloud/functions/internal/parse_options.h"
#include <boost/program_options.hpp>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace google::cloud::functions_internal {
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_BEGIN
namespace po = boost::program_options;

auto constexpr kDefaultPort = 8080;

po::variables_map ParseOptions(int argc, char const* const argv[]) {
  // Initialize the default port with the value from the "PORT" environment
  // variable or with 8080.
  auto port = [&]() {
    auto const* env = std::getenv("PORT");
    if (env == nullptr) return kDefaultPort;
    return std::stoi(env);
  }();

  // Parse the command-line options.
  po::options_description desc("Server configuration");
  desc.add_options()
      //
      ("help", "produce help message")
      //
      ("address", po::value<std::string>()->default_value("0.0.0.0"),
       "set listening address")
      //
      ("target", po::value<std::string>()->default_value(""),
       "Ignored. This is required by the Functions Framework contract."
       " See https://github.com/GoogleCloudPlatform/functions-framework for"
       " additional information.")
      //
      ("signature-type", po::value<std::string>()->default_value(""),
       "Ignored. This is required by the Functions Framework contract."
       " See https://github.com/GoogleCloudPlatform/functions-framework for"
       " additional information.")
      //
      ("port", po::value<int>()->default_value(port), "set listening port");
  po::variables_map vm;
  char const* a[] = {"missing-command"};
  // Boost.Options throws an exception if argc == 0, we want to avoid that.
  auto ac = argc == 0 ? 1 : argc;
  auto const* av = argc == 0 ? a : argv;
  po::store(po::parse_command_line(ac, av, desc), vm);
  po::notify(vm);
  if (vm.count("help") != 0) {
    std::cout << desc << "\n";
  }
  auto port_value = vm["port"].as<int>();
  if (port_value >= std::numeric_limits<std::uint16_t>::min() &&
      port_value <= std::numeric_limits<std::uint16_t>::max()) {
    return vm;
  }
  std::ostringstream os;
  os << "The configured port (" << port_value << ") is out of range.";
  throw std::invalid_argument(std::move(os).str());
}

FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_END
}  // namespace google::cloud::functions_internal
