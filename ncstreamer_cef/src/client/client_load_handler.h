/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_CLIENT_CLIENT_LOAD_HANDLER_H_
#define NCSTREAMER_CEF_SRC_CLIENT_CLIENT_LOAD_HANDLER_H_


#include <string>
#include <vector>

#include "include/cef_load_handler.h"


namespace ncstreamer {
class ClientLoadHandler : public CefLoadHandler {
 public:
  ClientLoadHandler(bool shows_sources_all,
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
  static std::vector<std::string> FilterSources(
      const std::vector<std::string> &all,
      const std::vector<std::string> &filter);

  void OnMainBrowserCreated(CefRefPtr<CefBrowser> browser);

  void UpdateSourcesPeriodically(
      CefRefPtr<CefBrowser> browser,
      int64_t millisec);

  const bool shows_sources_all_;
  const std::vector<std::string> white_sources_;
  std::vector<std::string> prev_sources_;

  bool main_browser_created_;

  IMPLEMENT_REFCOUNTING(ClientLoadHandler);
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_CLIENT_CLIENT_LOAD_HANDLER_H_
