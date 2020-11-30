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

#ifndef FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_CLOUD_EVENT_H
#define FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_CLOUD_EVENT_H

#include "google/cloud/functions/version.h"
#include <chrono>
#include <optional>
#include <string>

namespace google::cloud::functions {
inline namespace FUNCTIONS_FRAMEWORK_CPP_NS {

/**
 * Represents a Cloud Event.
 *
 * Functions to handle Cloud Events receive an object of this class.
 *
 * @see The [Cloud Events Primer][cloud-events-primer] for a general
 *     introduction to Cloud Events.
 *
 * [cloud-events-primer]: https://github.com/cloudevents/spec
 */
class CloudEvent {
 public:
  inline static auto constexpr kDefaultSpecVersion = "1.0";
  using ClockType = std::chrono::system_clock;
  using time_point = ClockType::time_point;

  CloudEvent(std::string id, std::string source, std::string type,
             std::string spec_version = kDefaultSpecVersion)
      : id_(std::move(id)),
        source_(std::move(source)),
        type_(std::move(type)),
        spec_version_(std::move(spec_version)) {}

  [[nodiscard]] std::string id() const { return id_; }
  [[nodiscard]] std::string source() const { return source_; }
  [[nodiscard]] std::string type() const { return type_; }
  [[nodiscard]] std::string spec_version() const { return spec_version_; }

  [[nodiscard]] std::optional<std::string> data_content_type() const {
    return data_content_type_;
  }
  [[nodiscard]] std::optional<std::string> data_schema() const {
    return data_schema_;
  }
  [[nodiscard]] std::optional<std::string> subject() const { return subject_; }
  [[nodiscard]] std::optional<time_point> time() const { return time_; }
  [[nodiscard]] std::optional<std::string> data() const& { return data_; }
  [[nodiscard]] std::optional<std::string> data() && {
    return std::move(data_);
  }

  void set_data_content_type(std::string v) {
    data_content_type_ = std::move(v);
  }
  void reset_data_content_type() { data_content_type_.reset(); }

  void set_data_schema(std::string v) { data_schema_ = std::move(v); }
  void reset_data_schema() { data_schema_.reset(); }

  void set_subject(std::string v) { subject_ = std::move(v); }
  void reset_subject() { subject_.reset(); }

  void set_time(time_point tp) { time_ = tp; }
  void set_time(std::string const& timestamp);
  void reset_time() { time_.reset(); }

  void set_data(std::string v) { data_ = std::move(v); }
  void reset_data() { data_.reset(); }

 private:
  std::string const id_;
  std::string const source_;
  std::string const type_;
  std::string const spec_version_;

  std::optional<std::string> data_content_type_;
  std::optional<std::string> data_schema_;
  std::optional<std::string> subject_;
  std::optional<time_point> time_;
  std::optional<std::string> data_;
};

}  // namespace FUNCTIONS_FRAMEWORK_CPP_NS
}  // namespace google::cloud::functions

#endif  // FUNCTIONS_FRAMEWORK_CPP_GOOGLE_CLOUD_FUNCTIONS_CLOUD_EVENT_H
