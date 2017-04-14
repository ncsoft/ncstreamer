/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_BROWSER_APP_H_
#define NCSTREAMER_CEF_SRC_BROWSER_APP_H_


#include <string>
#include <vector>

#include "include/cef_app.h"
#include "windows.h"  // NOLINT

#include "ncstreamer_cef/src/browser_process_handler.h"


namespace ncstreamer {
class BrowserApp : public CefApp {
 public:
  BrowserApp(
      HINSTANCE instance,
      bool shows_sources_all,
      const std::vector<std::string> &sources,
      const std::wstring &locale);
  virtual ~BrowserApp();

  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override;

 private:
  CefRefPtr<BrowserProcessHandler> browser_process_handler_;

  IMPLEMENT_REFCOUNTING(BrowserApp);
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_BROWSER_APP_H_
