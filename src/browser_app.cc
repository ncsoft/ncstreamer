/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/browser_app.h"


namespace ncstreamer {
BrowserApp::BrowserApp(HINSTANCE instance)
    : browser_process_handler_{new BrowserProcessHandler{instance}} {
}


BrowserApp::~BrowserApp() {
}


CefRefPtr<CefBrowserProcessHandler> BrowserApp::GetBrowserProcessHandler() {
  return browser_process_handler_;
}
}  // namespace ncstreamer
