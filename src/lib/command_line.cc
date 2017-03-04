/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/lib/command_line.h"

#include "include/cef_command_line.h"


namespace ncstreamer {
CommandLine::CommandLine(const std::wstring &cmd_line)
    : is_renderer_{false} {
  CefRefPtr<CefCommandLine> cef_cmd_line =
      CefCommandLine::CreateCommandLine();
  cef_cmd_line->InitFromString(cmd_line);

  const std::wstring &process_type =
      cef_cmd_line->GetSwitchValue(L"type");
  is_renderer_ = (process_type == L"renderer");
}


CommandLine::~CommandLine() {
}
}  // namespace ncstreamer
