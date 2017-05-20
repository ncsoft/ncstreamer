/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/client/client_load_handler.h"

#include <cassert>
#include <sstream>
#include <unordered_map>

#include "include/wrapper/cef_helpers.h"

#include "ncstreamer_cef/src/js_executor.h"
#include "ncstreamer_cef/src/obs.h"
#include "ncstreamer_cef/src/obs/obs_source_info.h"


namespace ncstreamer {
ClientLoadHandler::ClientLoadHandler(
    bool shows_sources_all,
    const std::vector<std::string> &sources)
    : shows_sources_all_{shows_sources_all},
      sources_{sources},
      main_browser_created_{false} {
}


ClientLoadHandler::~ClientLoadHandler() {
}


void ClientLoadHandler::OnLoadStart(CefRefPtr<CefBrowser> /*browser*/,
                                    CefRefPtr<CefFrame> /*frame*/) {
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
  if (main_browser_created_ == false && isLoading == false) {
    ::ShowWindow(browser->GetHost()->GetWindowHandle(), TRUE);
    main_browser_created_ = true;
    OnMainBrowserCreated(browser);
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


void ClientLoadHandler::OnMainBrowserCreated(
    CefRefPtr<CefBrowser> browser) {
  const auto &all = Obs::Get()->FindAllWindowsOnDesktop();
  const auto &sources = (shows_sources_all_ == true) ?
      all : FilterSources(all, sources_);
  JsExecutor::Execute(browser, "updateStreamingSources", "sources", sources);
}
}  // namespace ncstreamer
