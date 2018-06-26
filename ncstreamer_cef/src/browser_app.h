/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_BROWSER_APP_H_
#define NCSTREAMER_CEF_SRC_BROWSER_APP_H_


#include <string>
#include <vector>

#include "boost/property_tree/ptree.hpp"
#include "include/cef_app.h"
#include "windows.h"  // NOLINT

#include "ncstreamer_cef/src/browser_process_handler.h"
#include "ncstreamer_cef/src/command_line.h"
#include "ncstreamer_cef/src/lib/position.h"
#include "ncstreamer_cef/src/streaming_service/streaming_service_types.h"


namespace ncstreamer {
class BrowserApp : public CefApp {
 public:
  BrowserApp(
      HINSTANCE instance,
      const CommandLine &cmd_line);
  virtual ~BrowserApp();

  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override;

  const CefRefPtr<CefBrowser> &GetMainBrowser() const;

 private:
  CefRefPtr<BrowserProcessHandler> browser_process_handler_;

  IMPLEMENT_REFCOUNTING(BrowserApp);
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_BROWSER_APP_H_
