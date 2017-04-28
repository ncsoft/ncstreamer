/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_CLIENT_H_
#define NCSTREAMER_CEF_SRC_CLIENT_H_


#include <string>
#include <vector>

#include "include/cef_client.h"
#include "windows.h"  // NOLINT

#include "ncstreamer_cef/src/client/client_display_handler.h"
#include "ncstreamer_cef/src/client/client_life_span_handler.h"
#include "ncstreamer_cef/src/client/client_load_handler.h"
#include "ncstreamer_cef/src/client/client_request_handler.h"


namespace ncstreamer {
class Client : public CefClient {
 public:
  Client(
      HINSTANCE instance,
      bool shows_sources_all,
      const std::vector<std::string> &sources,
      const std::wstring &locale);

  virtual ~Client();

  CefRefPtr<CefDisplayHandler> GetDisplayHandler() override;
  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override;
  CefRefPtr<CefLoadHandler> GetLoadHandler() override;
  CefRefPtr<CefRequestHandler> GetRequestHandler() override;

  const CefRefPtr<CefBrowser> &GetMainBrowser() const;

 private:
  CefRefPtr<ClientDisplayHandler> display_handler_;
  CefRefPtr<ClientLifeSpanHandler> life_span_handler_;
  CefRefPtr<ClientLoadHandler> load_handler_;
  CefRefPtr<ClientRequestHandler> request_handler_;

  IMPLEMENT_REFCOUNTING(Client);
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_CLIENT_H_
