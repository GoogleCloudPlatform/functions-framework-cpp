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

// [START functions_pubsub_integration_test]
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <curl/curl.h>
#include <gmock/gmock.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <string>

namespace {

namespace bp = boost::process;
namespace bfs = boost::filesystem;  // Boost.Process cannot use std::filesystem
using ::testing::HasSubstr;
using ::testing::IsEmpty;

struct HttpResponse {
  long code;  // NOLINT(google-runtime-int)
  std::string payload;
};

HttpResponse HttpEvent(std::string const& url, std::string const& payload);

char const* argv0 = nullptr;

class PubsubIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    curl_global_init(CURL_GLOBAL_ALL);

    auto const exe =
        bfs::path(argv0).parent_path() / "pubsub_integration_server";
    auto server =
        bp::child(exe, "--port=8081", (bp::std_out & bp::std_err) > child_log_);
    ASSERT_TRUE(WaitForServerReady("http://localhost:8081/"));
    process_ = std::move(server);
  }

  // Wait until an HTTP server starts responding.
  bool WaitForServerReady(std::string const& url);

  void TearDown() override {
    process_.terminate();
    process_.wait();
    curl_global_cleanup();
  }

  std::string NextChildLine() {
    std::string line;
    std::getline(child_log_, line);
    return line;
  }

 private:
  bp::ipstream child_log_;
  bp::child process_;
};

TEST_F(PubsubIntegrationTest, Basic) {
  auto constexpr kOkay = 200;

  auto const base = nlohmann::json::parse(R"js({
    "specversion": "1.0",
    "type": "google.cloud.pubsub.topic.v1.messagePublished",
    "source": "//pubsub.googleapis.com/projects/sample-project/topics/gcf-test",
    "id": "aaaaaa-1111-bbbb-2222-cccccccccccc",
    "time": "2020-09-29T11:32:00.000Z",
    "datacontenttype": "application/json",
    "data": {
      "subscription": "projects/sample-project/subscriptions/sample-subscription",
      "message": {
      }
    }
  })js");

  struct TestCases {
    std::string data;
    std::string expected;
  } cases[]{
      // The magic string was obtained using:
      //  /bin/echo -n 'C++' | openssl base64 -e
      {"Qysr", "Hello C++"},
      {"", "Hello World"},
  };

  for (auto const& test : cases) {
    SCOPED_TRACE("Testing for " + test.expected);
    auto object = base;
    object["data"]["message"] = nlohmann::json{{"data", test.data}};
    SCOPED_TRACE("event=" + object.dump());
    auto actual = HttpEvent("http://localhost:8081/", object.dump());
    EXPECT_EQ(actual.code, kOkay);
    EXPECT_THAT(actual.payload, IsEmpty());

    auto line = NextChildLine();
    EXPECT_THAT(line, HasSubstr(test.expected));
  }
}

extern "C" size_t CurlOnWriteData(char* ptr, size_t size, size_t nmemb,
                                  void* userdata) {
  auto* buffer = reinterpret_cast<std::string*>(userdata);
  buffer->append(ptr, size * nmemb);
  return size * nmemb;
}

HttpResponse HttpEvent(std::string const& url, std::string const& payload) {
  using CurlHandle = std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>;
  using CurlHeaders =
      std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)>;

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

  auto headers = CurlHeaders(nullptr, curl_slist_free_all);
  auto add_header = [&headers](std::string const& h) {
    auto* nh = curl_slist_append(headers.get(), h.c_str());
    (void)headers.release();
    headers.reset(nh);
  };

  setopt(CURLOPT_URL, url.c_str());
  setopt(CURLOPT_POSTFIELDSIZE, payload.size());
  setopt(CURLOPT_POSTFIELDS, payload.data());
  setopt(CURLOPT_WRITEFUNCTION, &CurlOnWriteData);
  add_header("Content-Type: application/cloudevents+json");
  add_header("Content-Length: " + std::to_string(payload.size()));
  setopt(CURLOPT_HTTPHEADER, headers.get());
  std::string buffer;
  setopt(CURLOPT_WRITEDATA, &buffer);

  auto e = curl_easy_perform(easy.get());
  if (e == CURLE_OK) {
    return HttpResponse{get_response_code(), std::move(buffer)};
  }
  return HttpResponse{-1, {}};
}

bool PubsubIntegrationTest::WaitForServerReady(std::string const& url) {
  using namespace std::chrono_literals;
  auto constexpr kOkay = 200;
  for (auto delay : {100ms, 200ms, 400ms, 800ms, 1600ms}) {  // NOLINT
    std::this_thread::sleep_for(delay);
    auto constexpr kPing = R"js({
      "specversion": "1.0",
      "type": "test-type",
      "source": "test-source",
      "id": "test-id",
      "datacontenttype": "application/json",
      "data": {
        "message": { "data": "" }
      }
    })js";
    auto r = HttpEvent(url, kPing);
    auto line = NextChildLine();
    if (r.code == kOkay) return true;
    std::cerr << "... [" << r.code << "] " << line << std::endl;
  }
  return false;
}

}  // namespace

int main(int argc, char* argv[]) {
  ::testing::InitGoogleMock(&argc, argv);
  ::argv0 = argv[0];
  return RUN_ALL_TESTS();
}

// [END functions_pubsub_integration_test]
