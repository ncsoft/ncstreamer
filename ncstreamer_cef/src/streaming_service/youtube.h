/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#ifndef NCSTREAMER_CEF_SRC_STREAMING_SERVICE_YOUTUBE_H_
#define NCSTREAMER_CEF_SRC_STREAMING_SERVICE_YOUTUBE_H_

#include <mutex>  // NOLINT
#include <string>

#include "boost/date_time/posix_time/posix_time.hpp"

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

  using OnBroadcastGotten = std::function<void(
      const std::string &broadcast_id,
      const std::string &stream_id,
      const std::string &page_link)>;

  using OnStreamGotten = std::function<void(
      const std::string &stream_server,
      const std::string &stream_key)>;

  using OnBroadcastUpdated = std::function<void()>;

  using OnStreamActiveChecked = std::function<void()>;

  class LoginClient;

  void GetRefreshToken(const std::string &last_access_token);

  void GetChannel(
      const OnFailed &on_failed,
      const OnChannelGotten &on_channel_gotten);

  void GetBroadcast(
      const OnFailed &on_failed,
      const OnBroadcastGotten &on_braodcast_gotten);

  void GetStream(
      const std::string &stream_id,
      const OnFailed &on_failed,
      const OnStreamGotten &on_stream_gotten);

  void UpdateBroadcast(
      const std::string &broadcast_id,
      const std::string &title,
      const std::string &privacy_status,
      const OnFailed &on_failed,
      const OnBroadcastUpdated &on_broadcast_updated);

  void OnLoginSuccess(
      const std::string &code,
      const OnFailed &on_failed,
      const OnLoggedIn &on_logged_in);

  std::string GetAccessToken() const;
  void SetAccessToken(const std::string &access_token);

  std::string GetVideoId() const;
  void SetVideoId(const std::string &video_id);

  std::string GetLiveChatId() const;
  void SetLiveChatId(const std::string &live_chat_id);

  boost::posix_time::ptime RefineISOTimeString(const std::string &time) const;

  const std::string ExtractLinkFromHtml(const std::string &html);

  CefRefPtr<LoginClient> login_client_;
  HttpRequestService http_request_service_;

  mutable std::mutex access_token_mutex_;
  std::string access_token_;

  std::string refresh_token_;

  mutable std::mutex live_chat_id_mutex_;
  std::string live_chat_id_;

  mutable std::mutex video_id_mutex_;
  std::string video_id_;
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
