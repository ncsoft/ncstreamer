/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/app.h"


namespace ncstreamer {
App::App(HINSTANCE instance)
    : browser_process_handler_{new BrowserProcessHandler{instance}} {
}


App::~App() {
}


CefRefPtr<CefBrowserProcessHandler> App::GetBrowserProcessHandler() {
  return browser_process_handler_;
}
}  // namespace ncstreamer
