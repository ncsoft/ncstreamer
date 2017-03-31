/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_STREAMING_SERVICE_FACEBOOK_H_
#define SRC_STREAMING_SERVICE_FACEBOOK_H_


#include <string>
#include <unordered_map>
#include <vector>

#include "boost/property_tree/ptree.hpp"
#include "include/cef_life_span_handler.h"
#include "include/cef_request_handler.h"

#include "src/lib/cef_fit_client.h"
#include "src/lib/http_request_service.h"
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

  void PostLiveVideo(
      const std::wstring &user_page_id,
      const std::wstring &privacy,
      const std::wstring &title,
      const std::wstring &description,
      const OnFailed &on_failed,
      const OnLiveVideoPosted &on_live_video_posted) override;

 private:
  using AccountMap =
      std::unordered_map<std::wstring /*id*/, UserPage>;

  using OnMeGotten = std::function<void(
      const std::wstring &me_id,
      const std::wstring &me_name,
      const std::vector<UserPage> &me_accounts)>;

  class LoginClient;

  static std::vector<UserPage> ExtractAccountAll(
      const boost::property_tree::ptree &tree);

  void GetMe(
      const OnFailed &on_failed,
      const OnMeGotten &on_me_gotten);

  void OnLoginSuccess(
      const std::wstring &access_token,
      const OnFailed &on_failed,
      const OnLoggedIn &on_logged_in);

  CefRefPtr<LoginClient> login_client_;
  HttpRequestService http_request_service_;

  std::wstring access_token_;
  std::wstring me_id_;
  std::wstring me_name_;
  AccountMap me_accounts_;
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
  bool OnLoginSuccess(
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


#endif  // SRC_STREAMING_SERVICE_FACEBOOK_H_
