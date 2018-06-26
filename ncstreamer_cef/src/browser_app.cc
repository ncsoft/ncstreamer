/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/browser_app.h"


namespace ncstreamer {
BrowserApp::BrowserApp(
    HINSTANCE instance,
    const CommandLine &cmd_line)
    : browser_process_handler_{new BrowserProcessHandler{
          instance,
          cmd_line}} {
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
