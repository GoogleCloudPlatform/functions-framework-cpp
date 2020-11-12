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
#include <gmock/gmock.h>
#include <cstdlib>

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {
namespace {

TEST(WrapRequestTest, NoCmd) {
  char const* argv[] = {};
  auto const vm = ParseOptions(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_EQ(vm.count("help"), 0);
}

TEST(WrapRequestTest, NoArgs) {
  char const* argv[] = {"unused"};
  auto const vm = ParseOptions(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_EQ(vm.count("help"), 0);
}

TEST(WrapRequestTest, Help) {
  char const* argv[] = {"unused", "--help"};
  auto const vm = ParseOptions(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_NE(vm.count("help"), 0);
}

TEST(WrapRequestTest, ExceptionOnUnknown) {
  char const* argv[] = {
      "unused",
      "--invalid-option-never-named-an-option-this",
  };
  int argc = sizeof(argv) / sizeof(argv[0]);
  EXPECT_THROW(ParseOptions(argc, argv), std::exception);
}

TEST(WrapRequestTest, AcceptsRequiredOptions) {
  ::unsetenv("PORT");
  char const* argv[] = {
      "unused",       "--port=8085",          "--address=localhost",
      "--target=foo", "--signature-type=bar",
  };
  auto const vm = ParseOptions(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_EQ(vm.count("help"), 0);
  EXPECT_EQ(vm["port"].as<int>(), 8085);
  EXPECT_EQ(vm["address"].as<std::string>(), "localhost");
  EXPECT_EQ(vm["target"].as<std::string>(), "foo");
  EXPECT_EQ(vm["signature-type"].as<std::string>(), "bar");
}

TEST(WrapRequestTest, UseEnvForPort) {
  ::setenv("PORT", "8081", 1);
  char const* argv[] = {"unused"};
  auto const vm = ParseOptions(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_EQ(vm["port"].as<int>(), 8081);
}

TEST(WrapRequestTest, CommandLineOverridesEnv) {
  ::setenv("PORT", "8081", 1);
  char const* argv[] = {"unused", "--port=8082"};
  auto const vm = ParseOptions(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_EQ(vm["port"].as<int>(), 8082);
}

TEST(WrapRequestTest, PortEnvInvalid) {
  ::setenv("PORT", "not-a-number", 1);
  char const* argv[] = {"unused"};
  int argc = sizeof(argv) / sizeof(argv[0]);
  EXPECT_THROW(ParseOptions(argc, argv), std::exception);
}

TEST(WrapRequestTest, PortEnvTooLow) {
  ::setenv("PORT", "-1", 1);
  char const* argv[] = {"unused"};
  int argc = sizeof(argv) / sizeof(argv[0]);
  EXPECT_THROW(ParseOptions(argc, argv), std::exception);
}

TEST(WrapRequestTest, PortEnvTooHigh) {
  ::setenv("PORT", "65536", 1);
  char const* argv[] = {"unused"};
  int argc = sizeof(argv) / sizeof(argv[0]);
  EXPECT_THROW(ParseOptions(argc, argv), std::exception);
}

TEST(WrapRequestTest, PortCommandLineInvalid) {
  ::unsetenv("PORT");
  char const* argv_1[] = {"unused", "--port=invalid-not-a-number"};
  char const* argv_2[] = {"unused", "--port=-1"};
  char const* argv_3[] = {"unused", "--port=65536"};

  EXPECT_THROW(ParseOptions(sizeof(argv_1) / sizeof(argv_1[0]), argv_1),
               std::exception);
  EXPECT_THROW(ParseOptions(sizeof(argv_2) / sizeof(argv_2[0]), argv_2),
               std::exception);
  EXPECT_THROW(ParseOptions(sizeof(argv_3) / sizeof(argv_3[0]), argv_3),
               std::exception);
}

}  // namespace
}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal
