/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#ifndef NCSTREAMER_CEF_SRC_STREAMING_SERVICE_YOUTUBE_H_
#define NCSTREAMER_CEF_SRC_STREAMING_SERVICE_YOUTUBE_H_

#include <mutex>  // NOLINT
#include <string>

#include "ncstreamer_cef/src/lib/cef_fit_client.h"
#include "ncstreamer_cef/src/lib/http_request_service.h"
#include "ncstreamer_cef/src/lib/uri.h"
#include "ncstreamer_cef/src/streaming_service/streaming_service_provider.h"


namespace ncstreamer {
class YouTube : public StreamingServiceProvider {
 public:
  YouTube();
  virtual ~YouTube();

  void LogIn(
      HWND parent,
      const std::wstring &locale,
      const OnFailed &on_failed,
      const OnLoggedIn &on_logged_in) override;

  void LogOut(
      const OnFailed &on_failed,
      const OnLoggedOut &on_logged_out) override;

  void PostLiveVideo(
      const std::string &stream_server,
      const std::string &user_page_id,
      const std::string &privacy,
      const std::string &title,
      const std::string &description,
      const std::string &app_attribution_tag,
      const OnFailed &on_failed,
      const OnLiveVideoPosted &on_live_video_posted) override;

  void GetComments(
      const std::string &created_time,
      const OnFailed &on_failed,
      const OnCommentsGot &on_comments_got) override;

  void GetLiveVideoViewers(
      const OnFailed &on_failed,
      const OnLiveVideoViewers &on_live_video_viewers) override;

  void StopLiveVideo() override;

 private:
  using OnChannelGotten = std::function<void(
      const std::string &user_name)>;

  class LoginClient;

  void GetRefreshToken();

  void GetChannel(
      const OnFailed &on_failed,
      const OnChannelGotten &on_channel_gotten);

  void OnLoginSuccess(
      const std::string &code,
      const OnFailed &on_failed,
      const OnLoggedIn &on_logged_in);

  std::string GetAccessToken() const;
  void SetAccessToken(const std::string &access_token);

  CefRefPtr<LoginClient> login_client_;
  HttpRequestService http_request_service_;

  mutable std::mutex access_token_mutex_;
  std::string access_token_;

  std::string refresh_token_;
};


class YouTube::LoginClient
    : public CefFitClient,
      public CefLifeSpanHandler,
      public CefRequestHandler {

 public:
  LoginClient(
      YouTube *const owner,
      const HWND &base_window,
      const OnFailed &on_failed,
      const OnLoggedIn &on_logged_in);
  virtual ~LoginClient();

 protected:
  // overrides CefClient
  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override;
  CefRefPtr<CefRequestHandler> GetRequestHandler() override;

  // overrides CefLifeSpanHandler
  void OnAfterCreated(
      CefRefPtr<CefBrowser> browser) override;

  // overrides CefRequestHandler
  bool OnBeforeBrowse(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefRefPtr<CefRequest> request,
      bool is_redirect) override;

 private:
  bool OnLoginRedirected(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefRefPtr<CefRequest> request,
      bool is_redirect,
      const Uri &uri);

  YouTube *const owner_;
  const HWND base_window_;

  OnFailed on_failed_;
  OnLoggedIn on_logged_in_;

  IMPLEMENT_REFCOUNTING(LoginClient);
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_STREAMING_SERVICE_YOUTUBE_H_
