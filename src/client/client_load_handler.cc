/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/client/client_load_handler.h"

#include <codecvt>
#include <locale>
#include <sstream>
#include <string>

#include "include/wrapper/cef_helpers.h"

#include "src/js_executor.h"
#include "src/obs.h"


namespace ncstreamer {
ClientLoadHandler::ClientLoadHandler()
    : main_browser_created_{false} {
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

  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::string utf8_err = converter.to_bytes(error_text);
  std::string utf8_url = converter.to_bytes(failed_url);

  std::stringstream ss;
  ss << "<html><body bgcolor=\"white\">"
     << "<h2>Failed to load URL " << utf8_url
     << " with error " << utf8_err
     << " (" << error_code
     << ").</h2></body></html>";

  frame->LoadString(ss.str(), utf8_url);
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
  const auto &sources = Obs::Get()->FindAllWindowsOnDesktop();
  JsExecutor::Execute(browser, "setUpStreamingSources", "sources", sources);
}
}  // namespace ncstreamer
