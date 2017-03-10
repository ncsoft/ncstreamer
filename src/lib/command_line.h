/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_LIB_COMMAND_LINE_H_
#define SRC_LIB_COMMAND_LINE_H_


#include <string>
#include <vector>

#include "windows.h"  // NOLINT


namespace ncstreamer {
class CommandLine {
 public:
  explicit CommandLine(const std::wstring &cmd_line);
  virtual ~CommandLine();

  bool is_renderer() const { return is_renderer_; }
  bool needs_to_find_sources() const { return needs_to_find_sources_; }
  const std::vector<std::string> &sources() const { return sources_; }

 private:
  static std::vector<std::string>
      ParseSourcesArgument(const std::wstring &arg);

  static std::string GetWindowTitle(HWND wnd);
  static std::string GetWindowClass(HWND wnd);
  static std::string GetExeName(DWORD process_id);

  bool is_renderer_;
  bool needs_to_find_sources_;
  std::vector<std::string> sources_;
};
}  // namespace ncstreamer


#endif  // SRC_LIB_COMMAND_LINE_H_
