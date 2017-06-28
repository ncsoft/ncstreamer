/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/browser_app.h"


namespace ncstreamer {
BrowserApp::BrowserApp(
    HINSTANCE instance,
    bool hides_settings,
    const std::wstring &video_quality,
    bool shows_sources_all,
    const std::vector<std::string> &sources,
    const std::wstring &locale,
    const std::wstring &ui_uri,
    const Position<int> &default_position)
    : browser_process_handler_{new BrowserProcessHandler{
          instance,
          hides_settings,
          video_quality,
          shows_sources_all,
          sources,
          locale,
          ui_uri,
          default_position}} {
}


BrowserApp::~BrowserApp() {
}


CefRefPtr<CefBrowserProcessHandler> BrowserApp::GetBrowserProcessHandler() {
  return browser_process_handler_;
}


const CefRefPtr<CefBrowser> &BrowserApp::GetMainBrowser() const {
  return browser_process_handler_->GetMainBrowser();
}
}  // namespace ncstreamer
