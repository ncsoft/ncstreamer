/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/command_line.h"

#include <codecvt>
#include <locale>
#include <memory>

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "windows.h"  // NOLINT


namespace ncstreamer {
CommandLine::CommandLine(const std::wstring &cmd_line)
    : is_renderer_{false},
      hides_settings_{false},
      video_quality_{},
      shows_sources_all_{false},
      sources_{},
      streaming_service_tag_ids_{},
      locale_{},
      ui_uri_{},
      remote_port_{0},
      in_memory_local_storage_{false},
      designated_user_{},
      default_position_{CW_USEDEFAULT, CW_USEDEFAULT},
      device_settings_{},
      location_{} {
  CefRefPtr<CefCommandLine> cef_cmd_line =
      CefCommandLine::CreateCommandLine();
  cef_cmd_line->InitFromString(cmd_line);

  const std::wstring &process_type =
      cef_cmd_line->GetSwitchValue(L"type");
  is_renderer_ = (process_type == L"renderer");

  hides_settings_ = ReadBool(cef_cmd_line, L"hides-settings", false);

  const std::wstring &video_quality =
      cef_cmd_line->GetSwitchValue(L"video-quality");
  video_quality_ = video_quality.empty() ? L"medium" : video_quality;

  shows_sources_all_ = ReadBool(cef_cmd_line, L"shows-sources-all", false);

  const std::wstring &sources_arg =
      cef_cmd_line->GetSwitchValue(L"sources");
  if (sources_arg.empty() == false) {
    sources_ = ParseSourcesArgument(sources_arg);
  }

  const std::wstring &tag_ids_arg =
      cef_cmd_line->GetSwitchValue(L"tag-ids");
  if (tag_ids_arg.empty() == false) {
    streaming_service_tag_ids_ = ParseTagIdsArgument(tag_ids_arg);
  }

  const std::wstring &locale =
      cef_cmd_line->GetSwitchValue(L"locale");
  locale_ = FindLocaleFolder(locale) ? locale : L"ko-KR";

  ui_uri_ = cef_cmd_line->GetSwitchValue(L"ui-uri");

  const std::wstring &remote_port =
      cef_cmd_line->GetSwitchValue(L"remote-port");
  try {
    remote_port_ = static_cast<uint16_t>(std::stoi(remote_port));
  } catch (...) {
    remote_port_ = 9002;
  }

  in_memory_local_storage_ =
      ReadBool(cef_cmd_line, L"in-memory-local-storage", false);

  designated_user_ = cef_cmd_line->GetSwitchValue(L"designated-user");

  const std::wstring default_position =
      cef_cmd_line->GetSwitchValue(L"default-position");
  if (default_position.empty() == false) {
    default_position_ = ParseDefaultPosition(default_position);
  }

  const std::wstring device_settings =
      cef_cmd_line->GetSwitchValue(L"device-settings");
  if (device_settings.empty() == false) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::string utf8 = converter.to_bytes(device_settings);
    std::stringstream root_ss{utf8};
    try {
      boost::property_tree::read_json(root_ss, device_settings_);
    } catch (const std::exception &/*e*/) {
      assert(false);
      return;
    }
  }

  location_ = cef_cmd_line->GetSwitchValue(L"location");
}


CommandLine::~CommandLine() {
}


bool CommandLine::ReadBool(
    const CefRefPtr<CefCommandLine> &cmd_line,
    const std::wstring &key,
    const bool &default_value) {
  if (cmd_line->HasSwitch(key) == false) {
    return default_value;
  }

  const std::wstring &value = cmd_line->GetSwitchValue(key);
  if (value == L"" ||
      value == L"true") {
    return true;
  }

  if (value == L"false") {
    return false;
  }

  return default_value;
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


StreamingServiceTagMap
    CommandLine::ParseTagIdsArgument(const std::wstring &arg) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::string utf8 = converter.to_bytes(arg);

  StreamingServiceTagMap service_tag_ids;
  boost::property_tree::ptree root;
  std::stringstream root_ss{utf8};
  try {
    boost::property_tree::read_json(root_ss, root);
    for (const auto &service_elem : root) {
      const std::string &service_provider = service_elem.first;
      const boost::property_tree::ptree &service_obj = service_elem.second;
      SourceTagMap source_tag_ids;
      for (const auto &source_elem : service_obj) {
        const std::string &source_title = source_elem.first;
        const std::string &tag_id = source_elem.second.get_value<std::string>();
        source_tag_ids.emplace(source_title, tag_id);
      }
      service_tag_ids.emplace(service_provider, source_tag_ids);
    }
  } catch (const std::exception &/*e*/) {
    service_tag_ids.clear();
  }
  return service_tag_ids;
}


Position<int> CommandLine::ParseDefaultPosition(const std::wstring &arg) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::string utf8 = converter.to_bytes(arg);
  int x;
  int y;
  boost::property_tree::ptree root;
  std::stringstream root_ss{utf8};
  try {
    boost::property_tree::read_json(root_ss, root);
    x = root.get<int>("x");
    y = root.get<int>("y");
  }
  catch (const std::exception &/*e*/) {
    x = CW_USEDEFAULT;
    y = CW_USEDEFAULT;
  }
  return {x, y};
}


bool CommandLine::FindLocaleFolder(const std::wstring &locale) {
  if (locale.empty()) {
    return false;
  }

  boost::filesystem::path locale_folder{L"ui/" + locale};
  if (!boost::filesystem::is_directory(locale_folder)) {
    return false;
  }
  return true;
}
}  // namespace ncstreamer
