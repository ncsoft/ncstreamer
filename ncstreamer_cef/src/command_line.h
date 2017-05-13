/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_COMMAND_LINE_H_
#define NCSTREAMER_CEF_SRC_COMMAND_LINE_H_


#include <string>
#include <vector>


namespace ncstreamer {
class CommandLine {
 public:
  explicit CommandLine(const std::wstring &cmd_line);
  virtual ~CommandLine();

  bool is_renderer() const { return is_renderer_; }
  bool shows_sources_all() const { return shows_sources_all_; }
  const std::vector<std::string> &sources() const { return sources_; }
  const std::wstring &locale() const { return locale_; }
  const std::wstring &ui_uri() const { return ui_uri_; }
  uint16_t remote_port() const { return remote_port_; }

 private:
  static std::vector<std::string>
      ParseSourcesArgument(const std::wstring &arg);

  bool is_renderer_;
  bool shows_sources_all_;
  std::vector<std::string> sources_;
  std::wstring locale_;
  std::wstring ui_uri_;
  uint16_t remote_port_;
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_COMMAND_LINE_H_
