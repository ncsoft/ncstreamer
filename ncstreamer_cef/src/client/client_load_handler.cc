/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/client/client_load_handler.h"

#include <cassert>
#include <codecvt>
#include <locale>
#include <sstream>
#include <unordered_map>

#include "boost/property_tree/ptree.hpp"
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
    const std::vector<std::string> &sources)
    : life_span_handler_{life_span_handler},
      hides_settings_{hides_settings},
      video_quality_{video_quality},
      shows_sources_all_{shows_sources_all},
      white_sources_{sources},
      prev_sources_{},
      main_page_loaded_{false} {
  assert(life_span_handler);
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
    const std::vector<std::string> &all,
    const std::vector<std::string> &filter) {
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

  std::vector<std::string> filtered_sources;
  for (const auto &title : filter) {
    auto i = workspace.find(title);
    assert(i != workspace.end());
    const auto &sources = i->second;
    for (const auto &source : sources) {
      filtered_sources.emplace_back(source);
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

  JsExecutor::Execute(browser, "setUp", args);
}


void ClientLoadHandler::UpdateSourcesPeriodically(
    int64_t millisec) {
  auto browser = life_span_handler_->main_browser();
  if (!browser) {
    return;
  }

  const auto &all = Obs::Get()->FindAllWindowsOnDesktop();
  const auto &sources = (shows_sources_all_ == true) ?
      all : FilterSources(all, white_sources_);

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
