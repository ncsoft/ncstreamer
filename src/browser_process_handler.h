/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_BROWSER_PROCESS_HANDLER_H_
#define SRC_BROWSER_PROCESS_HANDLER_H_


#include "include/cef_browser_process_handler.h"
#include "windows.h"  // NOLINT


namespace ncstreamer {
class BrowserProcessHandler : public CefBrowserProcessHandler {
 public:
  explicit BrowserProcessHandler(HINSTANCE instance);
  virtual ~BrowserProcessHandler();

  // override CefBrowserProcessHandler
  void OnContextInitialized() override;

 private:
  const HINSTANCE instance_;

  IMPLEMENT_REFCOUNTING(BrowserProcessHandler);
};
}  // namespace ncstreamer


#endif  // SRC_BROWSER_PROCESS_HANDLER_H_
