/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_STREAMING_SERVICE_FACEBOOK_H_
#define SRC_STREAMING_SERVICE_FACEBOOK_H_


#include <string>
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
      public CefLoadHandler,
      public CefRequestHandler {
 public:
  FacebookClient();
  virtual ~FacebookClient();

  void SetHandlers(
      const OnFailed &on_failed,
      const OnLoggedIn &on_logged_in);

 protected:
  // overrides CefClient
  CefRefPtr<CefLoadHandler> GetLoadHandler() override;
  CefRefPtr<CefRequestHandler> GetRequestHandler() override;

  // overrides CefLoadHandler
  void OnLoadEnd(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      int httpStatusCode) override;

  // overrides CefRequestHandler
  bool OnBeforeBrowse(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefRefPtr<CefRequest> request,
      bool is_redirect) override;

 private:
  void GetMe(
      const CefRefPtr<CefFrame> &frame,
      const std::wstring &access_token);

  void OnGetMe(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      int http_status_code,
      const Uri &uri);

  bool OnAccessToken(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefRefPtr<CefRequest> request,
      bool is_redirect,
      const Uri &uri);

  std::wstring access_token_;
  std::wstring me_id_;
  std::wstring me_name_;

  OnFailed on_failed_;
  OnLoggedIn on_logged_in_;

  IMPLEMENT_REFCOUNTING(FacebookClient);
};
}  // namespace ncstreamer


#endif  // SRC_STREAMING_SERVICE_FACEBOOK_H_
