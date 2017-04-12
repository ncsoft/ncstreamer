/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/browser_app.h"


namespace ncstreamer {
BrowserApp::BrowserApp(
    HINSTANCE instance,
    bool shows_sources_all,
    const std::vector<std::string> &sources,
    const std::wstring &locale)
    : browser_process_handler_{new BrowserProcessHandler{
          instance,
          shows_sources_all,
          sources,
          locale}} {
}


BrowserApp::~BrowserApp() {
}


CefRefPtr<CefBrowserProcessHandler> BrowserApp::GetBrowserProcessHandler() {
  return browser_process_handler_;
}
}  // namespace ncstreamer
