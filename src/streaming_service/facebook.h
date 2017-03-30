/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_STREAMING_SERVICE_FACEBOOK_H_
#define SRC_STREAMING_SERVICE_FACEBOOK_H_


#include <string>
#include <unordered_map>
#include <vector>

#include "boost/property_tree/ptree.hpp"
#include "include/cef_request_handler.h"

#include "src/lib/cef_fit_client.h"
#include "src/lib/http_download_service.h"
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
      const std::wstring &description,
      const OnFailed &on_failed,
      const OnLiveVideoPosted &on_live_video_posted) override;

 private:
  using AccountMap =
      std::unordered_map<std::wstring /*id*/, UserPage>;

  class FacebookClient;

  static std::vector<UserPage> ExtractAccountAll(
      const boost::property_tree::ptree &tree);

  void GetMe();

  void SetHandlers(
      const OnFailed &on_failed,
      const OnLoggedIn &on_logged_in);

  void OnAccessToken(
      const std::wstring &access_token);

  CefRefPtr<FacebookClient> facebook_client_;
  HttpDownloadService http_download_service_;

  std::wstring access_token_;
  std::wstring me_id_;
  std::wstring me_name_;
  AccountMap me_accounts_;

  OnFailed on_failed_;
  OnLoggedIn on_logged_in_;
};


class Facebook::FacebookClient
    : public CefFitClient,
      public CefRequestHandler {
 public:
  explicit FacebookClient(Facebook *const owner);
  virtual ~FacebookClient();

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

  Facebook *const owner_;

  IMPLEMENT_REFCOUNTING(FacebookClient);
};
}  // namespace ncstreamer


#endif  // SRC_STREAMING_SERVICE_FACEBOOK_H_
