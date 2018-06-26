/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_COMMAND_LINE_H_
#define NCSTREAMER_CEF_SRC_COMMAND_LINE_H_


#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "boost/property_tree/ptree.hpp"

#include "include/cef_command_line.h"
#include "ncstreamer_cef/src/lib/position.h"
#include "ncstreamer_cef/src/streaming_service/streaming_service_types.h"


namespace ncstreamer {
class CommandLine {
 public:
  explicit CommandLine(const std::wstring &cmd_line);
  virtual ~CommandLine();

  bool is_renderer() const { return is_renderer_; }
  bool hides_settings() const { return hides_settings_; }
  const std::wstring &video_quality() const { return video_quality_; }
  bool shows_sources_all() const { return shows_sources_all_; }
  const std::vector<std::string> &sources() const { return sources_; }
  const StreamingServiceTagMap &streaming_service_tag_ids() const {
    return streaming_service_tag_ids_;
  }
  const std::wstring &locale() const { return locale_; }
  const std::wstring &ui_uri() const { return ui_uri_; }
  uint16_t remote_port() const { return remote_port_; }
  bool in_memory_local_storage() const { return in_memory_local_storage_; }
  const std::wstring &designated_user() const { return designated_user_; }
  const Position<int> &default_position() const {
    return default_position_;
  }
  const boost::property_tree::ptree &device_settings() const {
      return device_settings_;
  }
  const std::wstring &location() const {
      return location_;
  }

 private:
  static bool ReadBool(
      const CefRefPtr<CefCommandLine> &cmd_line,
      const std::wstring &key,
      const bool &default_value);

  static std::vector<std::string>
      ParseSourcesArgument(const std::wstring &arg);

  static StreamingServiceTagMap
      ParseTagIdsArgument(const std::wstring &arg);

  static Position<int> ParseDefaultPosition(const std::wstring &arg);

  bool is_renderer_;
  bool hides_settings_;
  std::wstring video_quality_;
  bool shows_sources_all_;
  std::vector<std::string> sources_;
  StreamingServiceTagMap streaming_service_tag_ids_;
  std::wstring locale_;
  std::wstring ui_uri_;
  uint16_t remote_port_;
  bool in_memory_local_storage_;
  std::wstring designated_user_;
  Position<int> default_position_;
  boost::property_tree::ptree device_settings_;
  std::wstring location_;
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_COMMAND_LINE_H_
