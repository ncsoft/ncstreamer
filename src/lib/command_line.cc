/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/lib/command_line.h"

#include <codecvt>
#include <locale>
#include <memory>

#ifdef _MSC_VER
#pragma warning(disable: 4819)
#endif
#include "boost/property_tree/json_parser.hpp"
#ifdef _MSC_VER
#pragma warning(default: 4819)
#endif

#include "boost/property_tree/ptree.hpp"
#include "include/cef_command_line.h"

#include "Psapi.h"  // NOLINT


namespace ncstreamer {
CommandLine::CommandLine(const std::wstring &cmd_line)
    : is_renderer_{false},
      needs_to_find_sources_{false},
      sources_{} {
  CefRefPtr<CefCommandLine> cef_cmd_line =
      CefCommandLine::CreateCommandLine();
  cef_cmd_line->InitFromString(cmd_line);

  const std::wstring &process_type =
      cef_cmd_line->GetSwitchValue(L"type");
  is_renderer_ = (process_type == L"renderer");

  static std::wstring kNeedsToFindSources{L"needs_to_find_sources"};
  if (cef_cmd_line->HasSwitch(kNeedsToFindSources) == true) {
    const std::wstring &needs_to_find_sources =
        cef_cmd_line->GetSwitchValue(kNeedsToFindSources);
    needs_to_find_sources_ = (needs_to_find_sources == L"") ||
                             (needs_to_find_sources == L"true");
  }

  const std::wstring &sources_arg =
      cef_cmd_line->GetSwitchValue(L"sources");
  if (sources_arg.empty() == false) {
    sources_ = ParseSourcesArgument(sources_arg);
  }
}


CommandLine::~CommandLine() {
}


std::vector<std::string>
    CommandLine::ParseSourcesArgument(const std::wstring &arg) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::string utf8 = converter.to_bytes(arg);

  std::vector<std::string> sources;
  boost::property_tree::ptree root;
  std::stringstream root_ss{utf8};
  try {
    boost::property_tree::read_json(root_ss, root);
    const auto &arr = root.get_child("sources", {});
    for (const auto &elem : arr) {
      const boost::property_tree::ptree &obj = elem.second;
      DWORD process_id =
          std::stoul(obj.get<std::string>("processId"));
      HWND window = reinterpret_cast<HWND>(
          std::stoull(obj.get<std::string>("window")));

      const auto &title = GetWindowTitle(window);
      const auto &clazz = GetWindowClass(window);
      const auto &exe = GetExeName(process_id);

      std::stringstream source;
      source << title << ":" << clazz << ":" << exe;
      sources.emplace_back(source.str());
    }
  } catch (const std::exception &/*e*/) {
    sources.clear();
  }
  return sources;
}


std::string CommandLine::GetWindowTitle(HWND wnd) {
  int len = ::GetWindowTextLength(wnd);
  std::unique_ptr<wchar_t> buf{new wchar_t[len + 1]};
  ::GetWindowText(wnd, buf.get(), len + 1);

  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.to_bytes(buf.get());
}


std::string CommandLine::GetWindowClass(HWND wnd) {
  static const std::size_t kBufSize{256 + 1};
  wchar_t buf[kBufSize];
  ::GetClassName(wnd, buf, kBufSize);

  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.to_bytes(buf);
}

std::string CommandLine::GetExeName(DWORD process_id) {
  static const std::size_t kBufSize{MAX_PATH};
  wchar_t buf[kBufSize];
  HANDLE process = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, process_id);
  ::GetProcessImageFileName(process, buf, kBufSize);
  ::CloseHandle(process);

  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.to_bytes(buf);
}
}  // namespace ncstreamer
