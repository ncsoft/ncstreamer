/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_CLIENT_CLIENT_LOAD_HANDLER_H_
#define NCSTREAMER_CEF_SRC_CLIENT_CLIENT_LOAD_HANDLER_H_


#include <string>
#include <vector>

#include "include/cef_load_handler.h"

#include "ncstreamer_cef/src/client/client_life_span_handler.h"


namespace ncstreamer {
class ClientLoadHandler : public CefLoadHandler {
 public:
  ClientLoadHandler(
      const ClientLifeSpanHandler *const life_span_handler,
      bool hides_settings,
      const std::wstring &video_quality,
      bool shows_sources_all,
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

  void OnMainPageLoaded(CefRefPtr<CefBrowser> browser);

  void UpdateSourcesPeriodically(
      int64_t millisec);

  const ClientLifeSpanHandler *const life_span_handler_;

  const bool hides_settings_;
  const std::wstring video_quality_;
  const bool shows_sources_all_;
  const std::vector<std::string> white_sources_;
  std::vector<std::string> prev_sources_;

  bool main_page_loaded_;

  IMPLEMENT_REFCOUNTING(ClientLoadHandler);
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_CLIENT_CLIENT_LOAD_HANDLER_H_
