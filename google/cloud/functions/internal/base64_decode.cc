// Copyright 2021 Google LLC
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

#include "google/cloud/functions/internal/base64_decode.h"
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <stdexcept>

namespace google::cloud::functions_internal {
FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_BEGIN

// NOLINTNEXTLINE(misc-no-recursion)
std::string Base64Decode(std::string const& base64) {
  if (base64.size() % 4 != 0) {
    // This should be uncommon, to avoid copying every time, pad only when
    // needed and try again.
    auto padded = base64;
    padded.append((4 - padded.size() % 4) % 4, '=');
    return Base64Decode(padded);
  }

  namespace bai = boost::archive::iterators;
  auto constexpr kBase64RawBits = 6;
  auto constexpr kBase64EncodedBits = 8;
  using Decoder =
      bai::transform_width<bai::binary_from_base64<std::string::const_iterator>,
                           kBase64EncodedBits, kBase64RawBits>;
  // While we know how much padding we added, there may have been some padding
  // there, just not enough. We need to determine the actual number of `=`
  // characters at the end of the string.
  auto pad_count = std::distance(base64.rbegin(),
                                 std::find_if(base64.rbegin(), base64.rend(),
                                              [](auto c) { return c != '='; }));
  if (pad_count > 2) {
    throw std::invalid_argument("Invalid base64 string <" + base64 + ">");
  }

  auto data = std::string{Decoder(base64.begin()), Decoder(base64.end())};
  for (; pad_count != 0; --pad_count) data.pop_back();
  return data;
}

FUNCTIONS_FRAMEWORK_CPP_INLINE_NAMESPACE_END
}  // namespace google::cloud::functions_internal
