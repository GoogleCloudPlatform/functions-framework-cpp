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

// [START functions_storage_integration_test]
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
using ::testing::Contains;
using ::testing::HasSubstr;
using ::testing::IsEmpty;

struct HttpResponse {
  long code;  // NOLINT(google-runtime-int)
  std::string payload;
};

HttpResponse HttpEvent(std::string const& url, std::string const& payload);

char const* argv0 = nullptr;

class StorageIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    curl_global_init(CURL_GLOBAL_ALL);

    static auto const kPath = std::vector<bfs::path>{
        bfs::canonical(argv0).make_preferred().parent_path()};
    auto const exe = bp::search_path("storage_integration_server", kPath);
    auto server =
        bp::child(exe, "--port=8050", (bp::std_out & bp::std_err) > child_log_);
    ASSERT_TRUE(WaitForServerReady("http://localhost:8050/"));
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
    std::cerr << " ... line=" << line << std::endl;
    return line;
  }

 private:
  bp::ipstream child_log_;
  bp::child process_;
};

TEST_F(StorageIntegrationTest, Basic) {
  auto constexpr kOkay = 200;

  auto const base = nlohmann::json::parse(R"js({
    "specversion": "1.0",
    "type": "google.cloud.storage.object.v1.finalized",
    "source": "//storage.googleapis.com/projects/_/buckets/some-bucket",
    "subject": "objects/folder/Test.cs",
    "id": "aaaaaa-1111-bbbb-2222-cccccccccccc",
    "time": "2020-09-29T11:32:00.000Z",
    "datacontenttype": "application/json",
    "data": {
      "kind": "storage#object",
      "bucket": "some-bucket",
      "name": "object-name",
      "generation": "1587627537231057",
      "timeCreated": "2020-04-23T07:38:57.230Z",
      "updated": "2020-04-23T07:38:57.230Z"
    }
  })js");

  struct TestCases {
    std::string name;
    std::string expected;
  } cases[]{
      {"object1.txt", "Object: object1.txt"},
      {"object/with/longer/name.txt", "Object: object/with/longer/name.txt"},
  };

  for (auto const& test : cases) {
    SCOPED_TRACE("Testing for " + test.expected);
    auto object = base;
    object["data"]["name"] = test.name;
    SCOPED_TRACE("event=" + object.dump());
    auto actual = HttpEvent("http://localhost:8050/", object.dump());
    ASSERT_EQ(actual.code, kOkay);
    EXPECT_THAT(actual.payload, IsEmpty());

    std::vector<std::string> lines;
    for (auto line = NextChildLine();
         line.find("Updated: ") == std::string::npos; line = NextChildLine()) {
      lines.push_back(std::move(line));
    }
    EXPECT_THAT(lines, Contains(HasSubstr(test.expected)));
  }
}

extern "C" size_t CurlOnWriteData(char* ptr, size_t size, size_t nmemb,
                                  void* userdata) {
  reinterpret_cast<std::string*>(userdata)->append(ptr, size * nmemb);
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
    if (e != CURLE_OK) std::runtime_error("Cannot get response code");
    return code;
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
  std::string buffer;
  setopt(CURLOPT_WRITEDATA, &buffer);
  setopt(CURLOPT_WRITEFUNCTION, &CurlOnWriteData);
  add_header("Content-Type: application/cloudevents+json");
  add_header("Content-Length: " + std::to_string(payload.size()));
  setopt(CURLOPT_HTTPHEADER, headers.get());

  auto e = curl_easy_perform(easy.get());
  if (e != CURLE_OK) throw std::runtime_error("Error in curl_easy_perform");
  return HttpResponse{get_response_code(), std::move(buffer)};
}

bool StorageIntegrationTest::WaitForServerReady(std::string const& url) {
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
        "kind": "storage#object",
        "bucket": "wait-for-server-ready",
        "name": "wait-for-server-ready",
        "generation": "1587627537231057",
        "id": "some-bucket/object-name/1587627537231057",
        "timeCreated": "2020-04-23T07:38:57.230Z",
        "updated": "2020-04-23T07:38:57.230Z"
      }
    })js";
    auto r = HttpEvent(url, kPing);
    if (r.code != kOkay) continue;
    for (auto line = NextChildLine(); !line.empty(); line = NextChildLine()) {
      if (line.find("Updated: ") != std::string::npos) return true;
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
// [END functions_storage_integration_test]
