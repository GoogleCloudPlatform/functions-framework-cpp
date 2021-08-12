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

#include "google/cloud/functions/internal/framework_impl.h"
#include "google/cloud/functions/internal/call_user_function.h"
#include "google/cloud/functions/internal/parse_options.h"
#include "google/cloud/functions/version.h"
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/program_options.hpp>
#include <functional>
#include <future>
#include <iostream>
#include <thread>

namespace google::cloud::functions_internal {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {
namespace {
namespace be = boost::beast;
namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;

template <typename UserFunction>
void HandleSession(tcp::socket socket, UserFunction const& user_function) {
  auto report_error = [](be::error_code ec, char const* what) {
    // TODO(#35) - maybe replace with Boost.Log
    std::cerr << what << ": " << ec.message() << "\n";
  };
  be::error_code ec;
  for (;;) {
    be::flat_buffer buffer;
    // Read a request
    BeastRequest request;
    be::http::read(socket, buffer, request, ec);
    if (ec == be::http::error::end_of_stream) break;
    if (ec) return report_error(ec, "read");
    auto const keep_alive = request.keep_alive();
    auto response = CallUserFunction(user_function, std::move(request));
    // Flush any buffered output, as the application may be shutdown immediately
    // after the HTTP response is sent.
    std::cout << std::flush;
    std::clog << std::flush;
    std::cerr << std::flush;
    // Send the response
    response.set(be::http::field::server, BOOST_BEAST_VERSION_STRING);
    response.prepare_payload();
    response.keep_alive(keep_alive);
    be::http::write(socket, response, ec);
    if (ec) return report_error(ec, "write");
  }
  socket.shutdown(tcp::socket::shutdown_send, ec);
}

template <typename UserFunction>
int RunForTestImpl(int argc, char const* const argv[], UserFunction&& function,
                   std::function<bool()> const& shutdown,
                   std::function<void(int)> const& actual_port) {
  auto vm = ParseOptions(argc, argv);
  if (vm.count("help") != 0) return 0;

  auto address = asio::ip::make_address(vm["address"].as<std::string>());
  auto port = vm["port"].as<int>();

  asio::io_context ioc{1};
  tcp::acceptor acceptor{ioc, {address, static_cast<std::uint16_t>(port)}};
  acceptor.listen(boost::asio::socket_base::max_connections);
  actual_port(acceptor.local_endpoint().port());

  auto handle_session =
      [h = std::forward<UserFunction>(function)](tcp::socket socket) {
        HandleSession(std::move(socket), h);
      };

  auto cleanup = [](std::vector<std::future<void>> sessions, auto wait) {
    std::vector<std::future<void>> running;
    for (auto& s : sessions) {
      if (s.wait_for(wait) == std::future_status::timeout) {
        running.push_back(std::move(s));
        continue;
      }
      s.get();
    }
    return running;
  };

  // Reaching this number of sessions triggers a cleanup of any sessions that
  // may have finished already. We allow up to 80 sessions to start without
  // blocking, this should be enough for Cloud Run and Cloud Functions:
  //     https://cloud.google.com/run/docs/about-concurrency#concurrency_values
  auto constexpr kSessionCleanupThreshold = 80;

  // If we reach this number of sessions we block until we are below the cleanup
  // threshold.
  auto constexpr kMaximumSessions = 160;

  std::vector<std::future<void>> sessions;
  while (!shutdown()) {
    if (sessions.size() >= kSessionCleanupThreshold) {
      sessions = cleanup(std::move(sessions), std::chrono::milliseconds(0));
    }
    while (sessions.size() >= kMaximumSessions) {
      sessions = cleanup(std::move(sessions), std::chrono::seconds(1));
    }
    auto socket = acceptor.accept(ioc);
    if (!socket.is_open()) break;
    // Run a thread per-session, transferring ownership of the socket
    sessions.push_back(
        std::async(std::launch::async, handle_session, std::move(socket)));
  }
  return 0;
}

template <typename UserFunction>
int RunImpl(int argc, char const* const argv[], UserFunction&& f) noexcept try {
  return RunForTestImpl(
      argc, argv, std::forward<UserFunction>(f), [] { return false; },
      [](int /*unused*/) {});
} catch (std::exception const& ex) {
  std::cerr << "Standard C++ exception thrown " << ex.what() << "\n";
  return 1;
} catch (...) {
  std::cerr << "Unknown exception thrown\n";
  return 1;
}

}  // namespace

int RunForTest(int argc, char const* const argv[],
               functions::UserHttpFunction handler,
               std::function<bool()> const& shutdown,
               std::function<void(int)> const& actual_port) {
  return RunForTestImpl(argc, argv, std::move(handler), shutdown, actual_port);
}

int RunForTest(int argc, char const* const argv[],
               functions::UserCloudEventFunction handler,
               std::function<bool()> const& shutdown,
               std::function<void(int)> const& actual_port) {
  return RunForTestImpl(argc, argv, std::move(handler), shutdown, actual_port);
}

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions_internal

namespace google::cloud::functions {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {

int Run(int argc, char const* const argv[], UserHttpFunction handler) noexcept {
  return functions_internal::RunImpl(argc, argv, std::move(handler));
}

int Run(int argc, char const* const argv[],
        UserCloudEventFunction handler) noexcept {
  return functions_internal::RunImpl(argc, argv, std::move(handler));
}

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions
