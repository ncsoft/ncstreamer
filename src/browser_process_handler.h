/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_BROWSER_PROCESS_HANDLER_H_
#define SRC_BROWSER_PROCESS_HANDLER_H_


#include <string>
#include <vector>

#include "include/cef_browser_process_handler.h"
#include "windows.h"  // NOLINT


namespace ncstreamer {
class BrowserProcessHandler : public CefBrowserProcessHandler {
 public:
  BrowserProcessHandler(HINSTANCE instance,
                        bool needs_to_find_sources,
                        const std::vector<std::string> &sources);
  virtual ~BrowserProcessHandler();

  // override CefBrowserProcessHandler
  void OnContextInitialized() override;

 private:
  const HINSTANCE instance_;
  const bool needs_to_find_sources_;
  const std::vector<std::string> sources_;

  IMPLEMENT_REFCOUNTING(BrowserProcessHandler);
};
}  // namespace ncstreamer


#endif  // SRC_BROWSER_PROCESS_HANDLER_H_
