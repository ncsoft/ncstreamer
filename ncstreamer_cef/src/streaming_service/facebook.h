/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_STREAMING_SERVICE_FACEBOOK_H_
#define NCSTREAMER_CEF_SRC_STREAMING_SERVICE_FACEBOOK_H_


#include <mutex>  // NOLINT
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "boost/property_tree/ptree.hpp"
#include "include/cef_life_span_handler.h"
#include "include/cef_request_handler.h"

#include "ncstreamer_cef/src/lib/cef_fit_client.h"
#include "ncstreamer_cef/src/lib/http_request_service.h"
#include "ncstreamer_cef/src/lib/uri.h"
#include "ncstreamer_cef/src/streaming_service/streaming_service_provider.h"


namespace ncstreamer {
class Facebook : public StreamingServiceProvider {
 public:
  Facebook();
  virtual ~Facebook();

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

  void StopLiveVideo() override;

 private:
  using AccountMap =
      std::unordered_map<std::string /*id*/, UserPage>;

  using MeInfo = std::tuple<
      std::string /*me_id*/,
      std::string /*me_name*/,
      AccountMap /*me_accounts*/>;

  using OnMeGotten = std::function<void(
      const std::string &me_id,
      const std::string &me_name,
      const std::vector<UserPage> &me_accounts)>;

  class LoginClient;

  static std::vector<UserPage> ExtractAccountAll(
      const boost::property_tree::ptree &tree);

  void GetMe(
      const OnFailed &on_failed,
      const OnMeGotten &on_me_gotten);

  void GetPostUrl(
      const std::string &stream_id,
      const std::string &stream_url,
      const OnFailed &on_failed,
      const OnLiveVideoPosted &on_live_video_posted);

  void OnLoginSuccess(
      const std::string &access_token,
      const OnFailed &on_failed,
      const OnLoggedIn &on_logged_in);

  std::string GetAccessToken() const;
  std::string GetPageAccessToken(const std::string &page_id) const;
  std::string GetStreamId() const;

  void SetAccessToken(const std::string &access_token);
  void SetMeInfo(const MeInfo &me_info);
  void SetStreamId(const std::string &stream_id);

  CefRefPtr<LoginClient> login_client_;
  HttpRequestService http_request_service_;

  // TODO(khpark): refactoring this by AccessToken class.
  mutable std::mutex access_token_mutex_;
  std::string access_token_;

  // TODO(khpark): refactoring this by MeInfo class.
  mutable std::mutex me_info_mutex_;
  MeInfo me_info_;

  mutable std::mutex stream_id_mutex_;
  std::string stream_id_;
};


class Facebook::LoginClient
    : public CefFitClient,
      public CefLifeSpanHandler,
      public CefRequestHandler {
 public:
  LoginClient(
      Facebook *const owner,
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

  Facebook *const owner_;
  const HWND base_window_;

  OnFailed on_failed_;
  OnLoggedIn on_logged_in_;

  IMPLEMENT_REFCOUNTING(LoginClient);
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_STREAMING_SERVICE_FACEBOOK_H_
