/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_BROWSER_APP_H_
#define SRC_BROWSER_APP_H_


#include "include/cef_app.h"
#include "windows.h"  // NOLINT

#include "src/browser_process_handler.h"


namespace ncstreamer {
class BrowserApp : public CefApp {
 public:
  BrowserApp(HINSTANCE instance,
             bool needs_to_find_sources);
  virtual ~BrowserApp();

  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override;

 private:
  CefRefPtr<BrowserProcessHandler> browser_process_handler_;

  IMPLEMENT_REFCOUNTING(BrowserApp);
};
}  // namespace ncstreamer


#endif  // SRC_BROWSER_APP_H_
