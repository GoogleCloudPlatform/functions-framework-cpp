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

// [START functions_http_integration_test]
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <curl/curl.h>
#include <gmock/gmock.h>
#include <chrono>
#include <memory>
#include <string>

namespace {

namespace bp = boost::process;
namespace bfs = boost::filesystem;  // Boost.Process cannot use std::filesystem

struct HttpResponse {
  long code;
  std::string payload;
};

// Wait until an HTTP server starts responding.
bool WaitForServerReady(std::string const& url);
HttpResponse HttpGet(std::string const& url, std::string const& payload);

char const* argv0 = nullptr;

class HttpIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    curl_global_init(CURL_GLOBAL_ALL);

    auto const exe = bfs::path(argv0).parent_path() / "http_integration_server";
    auto server = bp::child(exe, "--port=8080");
    ASSERT_TRUE(WaitForServerReady("http://localhost:8080/"));
    process_ = std::move(server);
  }

  void TearDown() override {
    process_.terminate();
    process_.wait();
    curl_global_cleanup();
  }

 private:
  bp::child process_;
};

TEST_F(HttpIntegrationTest, Basic) {
  auto constexpr kOkay = 200;

  auto actual = HttpGet("http://localhost:8080/", R"js({"name": "Foo"})js");
  EXPECT_EQ(actual.code, kOkay);
  EXPECT_EQ(actual.payload, "Hello Foo!");

  actual = HttpGet("http://localhost:8080/", R"js({})js");
  EXPECT_EQ(actual.code, kOkay);
  EXPECT_EQ(actual.payload, "Hello World!");
}

extern "C" size_t CurlOnWriteData(char* ptr, size_t size, size_t nmemb,
                                         void* userdata) {
  auto* buffer = reinterpret_cast<std::string*>(userdata);
  buffer->append(ptr, size * nmemb);
  return size * nmemb;
}

HttpResponse HttpGet(std::string const& url, std::string const& payload) {
  using CurlHandle = std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>;

  auto easy = CurlHandle(curl_easy_init(), curl_easy_cleanup);

  auto setopt = [h = easy.get()](auto opt, auto value) {
    if (auto e = curl_easy_setopt(h, opt, value); e != CURLE_OK) {
      std::ostringstream os;
      os << "error [" << e << "] setting curl_easy option <" << opt
         << ">=" << value;
      throw std::runtime_error(std::move(os).str());
    }
  };
  auto get_response_code = [h = easy.get()]() {
    long code;                        // NOLINT(google-runtime-int)
    auto e = curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &code);
    if (e == CURLE_OK) {
      return code;
    }
    throw std::runtime_error("Cannot get response code");
  };

  setopt(CURLOPT_URL, url.c_str());
  setopt(CURLOPT_POSTFIELDSIZE, payload.size());
  setopt(CURLOPT_POSTFIELDS, payload.data());
  setopt(CURLOPT_WRITEFUNCTION, &CurlOnWriteData);
  std::string buffer;
  setopt(CURLOPT_WRITEDATA, &buffer);

  auto e = curl_easy_perform(easy.get());
  if (e == CURLE_OK) {
    return HttpResponse{get_response_code(), std::move(buffer)};
  }
  return HttpResponse{-1, {}};
}

bool WaitForServerReady(std::string const& url) {
  using namespace std::chrono_literals;
  auto constexpr kOkay = 200;
  for (auto delay : {100ms, 200ms, 400ms, 800ms, 1600ms}) {  // NOLINT
    std::this_thread::sleep_for(delay);
    auto r = HttpGet(url, "{}");
    if (r.code == kOkay) return true;
  }
  return false;
}

}  // namespace

int main(int argc, char* argv[]) {
  ::testing::InitGoogleMock(&argc, argv);
  ::argv0 = argv[0];
  return RUN_ALL_TESTS();
}

// [END functions_http_integration_test]
