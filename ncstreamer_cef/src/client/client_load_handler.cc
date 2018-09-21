/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/client/client_load_handler.h"

#include <cassert>
#include <codecvt>
#include <locale>
#include <regex>  // NOLINT
#include <sstream>
#include <unordered_map>

#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

#include "ncstreamer_cef/src/js_executor.h"
#include "ncstreamer_cef/src/obs.h"
#include "ncstreamer_cef/src/obs/obs_source_info.h"


namespace ncstreamer {
ClientLoadHandler::ClientLoadHandler(
    const ClientLifeSpanHandler *const life_span_handler,
    bool hides_settings,
    const std::wstring &video_quality,
    bool shows_sources_all,
    const std::vector<std::string> &sources,
    const boost::property_tree::ptree &device_setting,
    const std::wstring &uid_hash)
    : life_span_handler_{life_span_handler},
      hides_settings_{hides_settings},
      video_quality_{video_quality},
      shows_sources_all_{shows_sources_all},
      white_sources_{},
      device_settings_{device_setting},
      prev_sources_{},
      uid_hash_{uid_hash},
      main_page_loaded_{false} {
  assert(life_span_handler);
  if (uid_hash_.empty()) {
    white_sources_ = sources;
  } else {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::string hash = converter.to_bytes(uid_hash);
    for (const auto &source : sources) {
      const std::string pattern{"((" + source + ").*(\\(" + hash + "\\)).*)"};
      white_sources_.emplace_back(pattern);
    }
  }
}


ClientLoadHandler::~ClientLoadHandler() {
}


void ClientLoadHandler::OnLoadStart(CefRefPtr<CefBrowser> /*browser*/,
                                    CefRefPtr<CefFrame> /*frame*/,
                                    TransitionType /*transition_type*/) {
}


void ClientLoadHandler::OnLoadEnd(CefRefPtr<CefBrowser> /*browser*/,
                                  CefRefPtr<CefFrame> /*frame*/,
                                  int /*httpStatusCode*/) {
}


void ClientLoadHandler::OnLoadError(CefRefPtr<CefBrowser> /*browser*/,
                                    CefRefPtr<CefFrame> frame,
                                    ErrorCode error_code,
                                    const CefString &error_text,
                                    const CefString &failed_url) {
  CEF_REQUIRE_UI_THREAD();

  if (error_code == ERR_ABORTED) {
    return;
  }

  std::wstringstream ss;
  ss << L"<html><body bgcolor=\"white\">"
     << L"<h2>Failed to load URL " << failed_url.c_str()
     << L" with error " << (error_text.c_str() ? error_text.c_str() : L"NULL")
     << L" (" << error_code
     << L").</h2></body></html>";

  frame->LoadString(ss.str(), failed_url);
}


void ClientLoadHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
                                             bool isLoading,
                                             bool /*canGoBack*/,
                                             bool /*canGoForward*/) {
  if (main_page_loaded_ == false && isLoading == false) {
    ::ShowWindow(browser->GetHost()->GetWindowHandle(), TRUE);
    main_page_loaded_ = true;
    OnMainPageLoaded(browser);
  }
}


std::vector<std::string> ClientLoadHandler::FilterSources(
    const std::wstring &uid_hash,
    const std::vector<std::string> &all,
    const std::vector<std::string> &filter) {
  std::vector<std::string> filtered_sources;
  if (uid_hash.empty()) {
    std::unordered_map<std::string /*title*/,
        std::vector<std::string /*source*/>> workspace;
    for (const auto &title : filter) {
      static const std::vector<std::string> kEmptySources{};
      workspace.emplace(title, kEmptySources);
    }

    for (const auto &source : all) {
      ObsSourceInfo source_info{source};
      const auto &title = source_info.title();

      auto i = workspace.find(title);
      if (i == workspace.end()) {
        continue;
      }
      auto *sources = &(i->second);
      sources->emplace_back(source);
    }

    for (const auto &title : filter) {
      auto i = workspace.find(title);
      assert(i != workspace.end());
      const auto &sources = i->second;
      for (const auto &source : sources) {
        filtered_sources.emplace_back(source);
      }
    }
  } else {
    for (const auto &source : all) {
      ObsSourceInfo source_info{source};
      const auto &title = source_info.title();
      for (const auto &i : filter) {
        const std::regex pattern{i};
        std::smatch matches;
        const bool &found = std::regex_search(title, matches, pattern);
        if (found) {
          filtered_sources.emplace_back(matches[0]);
          break;
        }
      }
    }
  }
  return filtered_sources;
}


void ClientLoadHandler::OnMainPageLoaded(
    CefRefPtr<CefBrowser> browser) {
  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

  boost::property_tree::ptree args;
  args.add("hidesSettings", hides_settings_);
  args.add("videoQuality", converter.to_bytes(video_quality_));
  args.add_child("deviceSettings", device_settings_);

  JsExecutor::Execute(browser, "setUpControls", args);
}


void ClientLoadHandler::UpdateSourcesPeriodically(
    int64_t millisec) {
  auto browser = life_span_handler_->main_browser();
  if (!browser) {
    return;
  }

  const auto &all = Obs::Get()->FindAllWindowsOnDesktop();
  const auto &sources = (shows_sources_all_ == true) ?
      all : FilterSources(uid_hash_, all, white_sources_);

  if (sources != prev_sources_) {
    JsExecutor::Execute(browser, "updateStreamingSources", "sources", sources);
    prev_sources_ = sources;
  }

  ::CefPostDelayedTask(
      TID_UI,
      base::Bind(&ClientLoadHandler::UpdateSourcesPeriodically, this, millisec),
      millisec);
}
}  // namespace ncstreamer
