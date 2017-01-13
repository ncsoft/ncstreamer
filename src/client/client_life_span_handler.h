/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_CLIENT_CLIENT_LIFE_SPAN_HANDLER_H_
#define SRC_CLIENT_CLIENT_LIFE_SPAN_HANDLER_H_

#include <unordered_map>

#include "include/cef_life_span_handler.h"
#include "windows.h"  // NOLINT


namespace ncstreamer {
class ClientLifeSpanHandler : public CefLifeSpanHandler {
 public:
  explicit ClientLifeSpanHandler(HINSTANCE instance);
  virtual ~ClientLifeSpanHandler();

  void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  bool DoClose(CefRefPtr<CefBrowser> browser) override;
  void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

 private:
  HICON icon_;

  std::unordered_map<int/*browser id*/, CefRefPtr<CefBrowser>> browsers_;
  CefRefPtr<CefBrowser> main_browser_;

  IMPLEMENT_REFCOUNTING(ClientLifeSpanHandler);
};
}  // namespace ncstreamer


#endif  // SRC_CLIENT_CLIENT_LIFE_SPAN_HANDLER_H_
