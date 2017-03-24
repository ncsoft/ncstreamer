/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_STREAMING_SERVICE_FACEBOOK_H_
#define SRC_STREAMING_SERVICE_FACEBOOK_H_


#include "include/cef_request_handler.h"

#include "src/lib/cef_fit_client.h"
#include "src/lib/uri.h"
#include "src/streaming_service/streaming_service_provider.h"


namespace ncstreamer {
class Facebook : public StreamingServiceProvider {
 public:
  Facebook();
  virtual ~Facebook();

  void LogIn(
      HWND parent,
      const OnFailed &on_failed,
      const OnLoggedIn &on_logged_in) override;

 private:
  class FacebookClient;

  CefRefPtr<FacebookClient> facebook_client_;
};


class Facebook::FacebookClient
    : public CefFitClient,
      public CefRequestHandler {
 public:
  FacebookClient();
  virtual ~FacebookClient();

  void SetHandlers(
      const OnFailed &on_failed,
      const OnLoggedIn &on_logged_in);

 protected:
  // overrides CefClient
  CefRefPtr<CefRequestHandler> GetRequestHandler() override;

  // overrides CefRequestHandler
  bool OnBeforeBrowse(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefRefPtr<CefRequest> request,
      bool is_redirect) override;

 private:
  bool OnAccessToken(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefRefPtr<CefRequest> request,
      bool is_redirect,
      const Uri &uri);

  std::wstring access_token_;

  OnFailed on_failed_;
  OnLoggedIn on_logged_in_;

  IMPLEMENT_REFCOUNTING(FacebookClient);
};
}  // namespace ncstreamer


#endif  // SRC_STREAMING_SERVICE_FACEBOOK_H_
