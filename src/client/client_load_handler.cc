/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/client/client_load_handler.h"

#include <sstream>
#include <string>
#include <vector>

#include "include/wrapper/cef_helpers.h"

#include "src/js_executor.h"
#include "src/obs.h"


namespace ncstreamer {
ClientLoadHandler::ClientLoadHandler(bool needs_to_find_sources)
    : needs_to_find_sources_{needs_to_find_sources},
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
     << L" with error " << error_text.c_str()
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


void ClientLoadHandler::OnMainBrowserCreated(
    CefRefPtr<CefBrowser> browser) {
  const auto &sources = (needs_to_find_sources_ == true) ?
      Obs::Get()->FindAllWindowsOnDesktop() :
      std::vector<std::string>{};
  JsExecutor::Execute(browser, "setUpStreamingSources", "sources", sources);
}
}  // namespace ncstreamer
