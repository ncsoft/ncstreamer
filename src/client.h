/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_CLIENT_H_
#define SRC_CLIENT_H_


#include "include/cef_client.h"
#include "windows.h"  // NOLINT

#include "src/client/client_display_handler.h"
#include "src/client/client_life_span_handler.h"
#include "src/client/client_load_handler.h"
#include "src/client/client_request_handler.h"


namespace ncstreamer {
class Client : public CefClient {
 public:
  explicit Client(HINSTANCE instance);
  virtual ~Client();

  CefRefPtr<CefDisplayHandler> GetDisplayHandler() override;
  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override;
  CefRefPtr<CefLoadHandler> GetLoadHandler() override;
  CefRefPtr<CefRequestHandler> GetRequestHandler() override;

 private:
  CefRefPtr<ClientDisplayHandler> display_handler_;
  CefRefPtr<ClientLifeSpanHandler> life_span_handler_;
  CefRefPtr<ClientLoadHandler> load_handler_;
  CefRefPtr<ClientRequestHandler> request_handler_;

  IMPLEMENT_REFCOUNTING(Client);
};
}  // namespace ncstreamer


#endif  // SRC_CLIENT_H_