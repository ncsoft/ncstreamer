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
      L"https",
      L"www.facebook.com",
      L"/v2.8/dialog/oauth",
      {{L"client_id", kNcStreamerAppId},
       {L"redirect_uri",
        L"https://www.facebook.com/connect/login_success.html"},
       {L"display", L"popup"}});

  const Rectangle &parent_rect = Windows::GetWindowRectangle(parent);
  const Rectangle &popup_rect = parent_rect.Center(429, 402);

  CefWindowInfo window_info;
  window_info.SetAsPopup(parent, L"Facebook Login");
  CefWindowRectangle::Reset(popup_rect, &window_info);

  CefBrowserSettings browser_settings;

  CefBrowserHost::CreateBrowser(
      window_info, facebook_client_, login_uri, browser_settings, NULL);
}


Facebook::FacebookClient::FacebookClient() {
}


Facebook::FacebookClient::~FacebookClient() {
}
}  // namespace ncstreamer
