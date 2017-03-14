/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_CLIENT_CLIENT_LOAD_HANDLER_H_
#define SRC_CLIENT_CLIENT_LOAD_HANDLER_H_


#include <string>
#include <vector>

#include "include/cef_load_handler.h"


namespace ncstreamer {
class ClientLoadHandler : public CefLoadHandler {
 public:
  ClientLoadHandler(bool needs_to_find_sources,
                    const std::vector<std::string> &sources);
  virtual ~ClientLoadHandler();

  void OnLoadStart(CefRefPtr<CefBrowser> browser,
                   CefRefPtr<CefFrame> frame) override;

  void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                 CefRefPtr<CefFrame> frame,
                 int httpStatusCode) override;

  void OnLoadError(CefRefPtr<CefBrowser> browser,
                   CefRefPtr<CefFrame> frame,
                   ErrorCode error_code,
                   const CefString &error_text,
                   const CefString &failed_url) override;

  void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
                            bool isLoading,
                            bool canGoBack,
                            bool canGoForward) override;

 private:
  void OnMainBrowserCreated(CefRefPtr<CefBrowser> browser);

  const bool needs_to_find_sources_;
  const std::vector<std::string> sources_;

  bool main_browser_created_;

  IMPLEMENT_REFCOUNTING(ClientLoadHandler);
};
}  // namespace ncstreamer


#endif  // SRC_CLIENT_CLIENT_LOAD_HANDLER_H_
