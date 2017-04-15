/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/lib/command_line.h"

#include <codecvt>
#include <locale>
#include <memory>

#include "boost/property_tree/ptree.hpp"
#include "include/cef_command_line.h"

#include "ncstreamer_cef/src/lib/json_parser.h"


namespace ncstreamer {
CommandLine::CommandLine(const std::wstring &cmd_line)
    : is_renderer_{false},
      shows_sources_all_{false},
      sources_{},
      locale_{} {
  CefRefPtr<CefCommandLine> cef_cmd_line =
      CefCommandLine::CreateCommandLine();
  cef_cmd_line->InitFromString(cmd_line);

  const std::wstring &process_type =
      cef_cmd_line->GetSwitchValue(L"type");
  is_renderer_ = (process_type == L"renderer");

  static std::wstring kShowsSourcesAll{L"shows_sources_all"};
  if (cef_cmd_line->HasSwitch(kShowsSourcesAll) == true) {
    const std::wstring &shows_sources_all =
        cef_cmd_line->GetSwitchValue(kShowsSourcesAll);
    shows_sources_all_ = (shows_sources_all == L"") ||
                         (shows_sources_all == L"true");
  }

  const std::wstring &sources_arg =
      cef_cmd_line->GetSwitchValue(L"sources");
  if (sources_arg.empty() == false) {
    sources_ = ParseSourcesArgument(sources_arg);
  }

  locale_ = cef_cmd_line->GetSwitchValue(L"locale");
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
      const auto &source = obj.get<std::string>("title");
      sources.emplace_back(source);
    }
  } catch (const std::exception &/*e*/) {
    sources.clear();
  }
  return sources;
}
}  // namespace ncstreamer
