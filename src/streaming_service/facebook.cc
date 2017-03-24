/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/streaming_service/facebook.h"

#include "include/cef_browser.h"

#include "src/lib/cef_types.h"
#include "src/lib/uri.h"
#include "src/lib/windows_types.h"


namespace ncstreamer {
Facebook::Facebook()
    : facebook_client_{new FacebookClient{}} {
}


Facebook::~Facebook() {
}


void Facebook::LogIn(
    HWND parent,
    const OnFailed &on_failed,
    const OnLoggedIn &on_logged_in) {
  static const wchar_t *kNcStreamerAppId{[]() {
    static const wchar_t *kProduction{
        L"1789696898019802"};
    static const wchar_t *kAlpha{
        L"1795379417451550"};
    return kAlpha;
  }()};

  const std::wstring &login_uri = Uri::ToString(
      L"https://www.facebook.com/v2.8/dialog/oauth",
      Uri::Query{{
          {L"client_id", kNcStreamerAppId},
          {L"redirect_uri", kLoginRedirectUri.uri_string()},
          {L"response_type", L"token"},
          {L"display", L"popup"}}});

  const Rectangle &parent_rect = Windows::GetWindowRectangle(parent);
  const Rectangle &popup_rect = parent_rect.Center(429, 402);

  CefWindowInfo window_info;
  window_info.SetAsPopup(parent, L"Facebook Login");
  CefWindowRectangle::Reset(popup_rect, &window_info);

  CefBrowserSettings browser_settings;

  facebook_client_->SetHandlers(on_failed, on_logged_in);
  CefBrowserHost::CreateBrowser(
      window_info, facebook_client_, login_uri, browser_settings, NULL);
}


const Uri Facebook::kLoginRedirectUri{
    L"https://www.facebook.com/connect/login_success.html"};


Facebook::FacebookClient::FacebookClient()
    : on_failed_{},
      on_logged_in_{} {
}


Facebook::FacebookClient::~FacebookClient() {
}


void Facebook::FacebookClient::SetHandlers(
    const OnFailed &on_failed,
    const OnLoggedIn &on_logged_in) {
  on_failed_ = on_failed;
  on_logged_in_ = on_logged_in;
}
}  // namespace ncstreamer
