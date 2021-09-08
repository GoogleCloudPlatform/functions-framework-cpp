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
#include <optional>
#include <string>
#include <thread>

namespace {

namespace bp = boost::process;
namespace bfs = boost::filesystem;  // Boost.Process cannot use std::filesystem

struct HttpResponse {
  long code;  // NOLINT(google-runtime-int)
  std::string payload;
};

// Wait until an HTTP server starts responding.
bool WaitForServerReady(std::string const& url);
HttpResponse HttpGet(std::string const& url, std::string const& payload);

char const* argv0 = nullptr;

auto ExePath(bfs::path const& filename) {
  static auto const kPath = std::vector<bfs::path>{
      bfs::canonical(argv0).make_preferred().parent_path()};
  return bp::search_path(filename, kPath);
}

class HttpIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    auto const* base_url = std::getenv("BASE_URL");
    if (base_url != nullptr) {
      url_ = base_url;
      return;
    }
    curl_global_init(CURL_GLOBAL_ALL);
    auto server = bp::child(ExePath("http_integration_server"), "--port=8030");
    url_ = "http://localhost:8030";
    ASSERT_TRUE(WaitForServerReady(url_));
    process_ = std::move(server);
  }

  void TearDown() override {
    if (process_.has_value()) {
      process_->terminate();
      process_->wait();
    }
    curl_global_cleanup();
  }

  [[nodiscard]] std::string const& url() const { return url_; }

 private:
  std::optional<bp::child> process_;
  std::string url_;
};

// [START functions_http_system_test]
TEST_F(HttpIntegrationTest, Basic) {
  auto constexpr kOkay = 200;

  auto actual = HttpGet(url(), R"js({"name": "Foo"})js");
  EXPECT_EQ(actual.code, kOkay);
  EXPECT_EQ(actual.payload, "Hello Foo!");

  actual = HttpGet(url(), R"js({})js");
  EXPECT_EQ(actual.code, kOkay);
  EXPECT_EQ(actual.payload, "Hello World!");
}
// [END functions_http_system_test]

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
    long code;  // NOLINT(google-runtime-int)
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
    std::cout << "Waiting for server to start [" << delay.count() << "ms]\n";
    std::this_thread::sleep_for(delay);
    try {
      auto r = HttpGet(url, "{}");
      if (r.code == kOkay) return true;
      std::cerr << "... [" << r.code << "]" << std::endl;
    } catch (std::exception const& ex) {
      // The HttpEvent() function may fail with an exception until the server is
      // ready. Log it to ease troubleshooting in the CI builds.
      std::cerr << "WaitForServerReady[" << delay.count()
                << "ms]: server ping failed with " << ex.what() << std::endl;
    }
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
