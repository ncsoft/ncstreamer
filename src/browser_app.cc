/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/browser_app.h"


namespace ncstreamer {
BrowserApp::BrowserApp(HINSTANCE instance,
                       bool needs_to_find_sources)
    : browser_process_handler_{new BrowserProcessHandler{
          instance,
          needs_to_find_sources}} {
}


BrowserApp::~BrowserApp() {
}


CefRefPtr<CefBrowserProcessHandler> BrowserApp::GetBrowserProcessHandler() {
  return browser_process_handler_;
}
}  // namespace ncstreamer
