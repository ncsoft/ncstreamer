/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_APP_H_
#define SRC_APP_H_


#include "include/cef_app.h"
#include "windows.h"  // NOLINT

#include "src/browser_process_handler.h"


namespace ncstreamer {
class App : public CefApp {
 public:
  explicit App(HINSTANCE instance);
  virtual ~App();

  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override;

 private:
  CefRefPtr<BrowserProcessHandler> browser_process_handler_;

  IMPLEMENT_REFCOUNTING(App);
};
}  // namespace ncstreamer


#endif  // SRC_APP_H_
