/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/streaming_service/facebook.h"

#include <cassert>
#include <codecvt>
#include <functional>
#include <locale>
#include <string>
#include <unordered_map>

#include "boost/property_tree/ptree.hpp"
#include "include/cef_browser.h"
#include "include/wrapper/cef_helpers.h"

#include "src/lib/cef_types.h"
#include "src/lib/http_types.h"
#include "src/lib/json_parser.h"
#include "src/lib/uri.h"
#include "src/lib/windows_types.h"
#include "src/streaming_service/facebook_api.h"


namespace ncstreamer {
Facebook::Facebook()
    : login_client_{},
      http_request_service_{},
      access_token_{},
      me_id_{},
      me_name_{},
      me_accounts_{},
      on_failed_{},
      on_logged_in_{} {
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

  static const Uri kLoginUri{FacebookApi::Login::Oauth::BuildUri(
      kNcStreamerAppId,
      FacebookApi::Login::Redirect::static_uri(),
      L"token",
      L"popup",
      {L"pages_show_list",
       L"publish_actions"})};

  const Rectangle &parent_rect = Windows::GetWindowRectangle(parent);
  const Rectangle &popup_rect = parent_rect.Center(429, 402);

  CefWindowInfo window_info;
  window_info.SetAsPopup(parent, L"Facebook Login");
  CefWindowRectangle::Reset(popup_rect, &window_info);

  CefBrowserSettings browser_settings;

  if (!login_client_) {
    login_client_ = new LoginClient{this, parent};
  }

  SetHandlers(on_failed, on_logged_in);
  CefBrowserHost::CreateBrowser(
      window_info,
      login_client_,
      kLoginUri.uri_string(),
      browser_settings,
      NULL);
}


void Facebook::PostLiveVideo(
    const std::wstring &user_page_id,
    const std::wstring &privacy,
    const std::wstring &title,
    const std::wstring &description,
    const OnFailed &on_failed,
    const OnLiveVideoPosted &on_live_video_posted) {
  Uri live_video_uri{FacebookApi::Graph::LiveVideos::BuildUri(
      user_page_id)};
  boost::property_tree::ptree post_content{
      FacebookApi::Graph::LiveVideos::BuildPostContent(
          access_token_,
          privacy,
          title,
          description)};

  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  http_request_service_.Post(
      converter.to_bytes(live_video_uri.uri_string()),
      post_content,
      [on_failed](const boost::system::error_code &ec) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring msg{converter.from_bytes(ec.message())};
    on_failed(msg);
  }, [on_failed, on_live_video_posted](const std::string &utf8) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring str = converter.from_bytes(utf8);

    boost::property_tree::ptree tree;
    std::stringstream ss{utf8};
    std::wstring stream_url{};
    try {
      boost::property_tree::read_json(ss, tree);
      stream_url = converter.from_bytes(tree.get<std::string>("stream_url"));
    } catch (const std::exception &/*e*/) {
      stream_url = L"";
    }

    if (stream_url.empty() == true) {
      std::wstringstream msg;
      msg << L"could not get stream_url from: " << str;
      on_failed(msg.str());
      return;
    }

    OutputDebugString((stream_url + L"/stream_url\r\n").c_str());

    on_live_video_posted(stream_url);
  });
}


std::vector<StreamingServiceProvider::UserPage>
    Facebook::ExtractAccountAll(
        const boost::property_tree::ptree &tree) {
  std::vector<UserPage> accounts;
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

  const auto &arr = tree.get_child("data");
  for (const auto &elem : arr) {
    const auto &account = elem.second;
    const auto &id = converter.from_bytes(account.get<std::string>("id"));
    const auto &name = converter.from_bytes(account.get<std::string>("name"));
    accounts.emplace_back(id, name);
  }

  return accounts;
}


void Facebook::GetMe(
    const OnFailed &on_failed,
    const OnMeGotten &on_me_gotten) {
  Uri me_uri{FacebookApi::Graph::Me::BuildUri(
      access_token_,
      {L"id",
       L"name",
       L"accounts"})};

  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  http_request_service_.Get(
      converter.to_bytes(me_uri.uri_string()),
      [on_failed](const boost::system::error_code &ec) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring msg{converter.from_bytes(ec.message())};
    on_failed(msg);
  }, [on_failed, on_me_gotten](const std::string &utf8) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring str = converter.from_bytes(utf8);

    boost::property_tree::ptree me;
    std::stringstream me_ss{utf8};
    std::wstring me_id{};
    std::wstring me_name{};
    std::vector<UserPage> me_accounts;
    try {
      boost::property_tree::read_json(me_ss, me);
      me_id = converter.from_bytes(me.get<std::string>("id"));
      me_name = converter.from_bytes(me.get<std::string>("name"));
      me_accounts = ExtractAccountAll(me.get_child("accounts"));
    } catch (const std::exception &/*e*/) {
      me_id = L"";
      me_name = L"";
      me_accounts.clear();
    }

    if (me_id.empty() == true) {
      std::wstringstream msg;
      msg << L"could not get me from: " << str;
      on_failed(msg.str());
      return;
    }

    on_me_gotten(me_id, me_name, me_accounts);
  });
}


void Facebook::SetHandlers(
    const OnFailed &on_failed,
    const OnLoggedIn &on_logged_in) {
  on_failed_ = on_failed;
  on_logged_in_ = on_logged_in;
}


void Facebook::OnLoginSuccess(const std::wstring &access_token) {
  access_token_ = access_token;

  GetMe(on_failed_, [this](
      const std::wstring &me_id,
      const std::wstring &me_name,
      const std::vector<UserPage> &me_accounts) {
    me_id_ = me_id;
    me_name_ = me_name;
    for (const auto &account : me_accounts) {
      me_accounts_.emplace(account.id(), account);
    }

    OutputDebugString((me_id_ + L"/id\r\n").c_str());
    OutputDebugString((me_name_ + L"/name\r\n").c_str());
    OutputDebugString(
        (std::to_wstring(me_accounts_.size()) + L"/accounts\r\n").c_str());

    on_logged_in_(me_name, me_accounts);
  });
}


Facebook::LoginClient::LoginClient(
    Facebook *const owner,
    const HWND &base_window)
    : owner_{owner},
      base_window_{base_window} {
}


Facebook::LoginClient::~LoginClient() {
}


CefRefPtr<CefLifeSpanHandler>
    Facebook::LoginClient::GetLifeSpanHandler() {
  return this;
}


CefRefPtr<CefRequestHandler>
    Facebook::LoginClient::GetRequestHandler() {
  return this;
}


void Facebook::LoginClient::OnAfterCreated(
    CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  HWND wnd = browser->GetHost()->GetWindowHandle();

  HICON icon = (HICON) ::SendMessage(
      base_window_, WM_GETICON, ICON_SMALL, (LPARAM) NULL);
  ::SendMessage(wnd, WM_SETICON, ICON_SMALL, (LPARAM) icon);
}


bool Facebook::LoginClient::OnBeforeBrowse(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request,
    bool is_redirect) {
  CEF_REQUIRE_UI_THREAD();

  if (frame->IsMain() == false) {
    return false;  // proceed navigation.
  }

  using Handler = std::function<bool(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefRefPtr<CefRequest> request,
      bool is_redirect,
      const Uri &uri)>;

  using Api = FacebookApi;
  using This = Facebook::LoginClient;

  static const std::unordered_map<std::wstring, Handler> kHandlers{
      {Api::Login::Redirect::static_uri().scheme_authority_path(),
       std::bind(&This::OnLoginSuccess, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3,
           std::placeholders::_4,
           std::placeholders::_5)}};

  Uri uri{request->GetURL()};
  OutputDebugString((uri.uri_string() + L"\r\n").c_str());

  auto i = kHandlers.find(uri.scheme_authority_path());
  if (i == kHandlers.end()) {
    return false;  // proceed navigation.
  }

  return i->second(browser, frame, request, is_redirect, uri);
}


bool Facebook::LoginClient::OnLoginSuccess(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> /*frame*/,
    CefRefPtr<CefRequest> /*request*/,
    bool /*is_redirect*/,
    const Uri &uri) {
  CEF_REQUIRE_UI_THREAD();

  const std::wstring &access_token =
      FacebookApi::Login::Redirect::ExtractAccessToken(
          Uri::Query{uri.fragment()});

  if (access_token.empty() == true) {
    assert(false);
    return false;  // proceed navigation.
  }

  owner_->OnLoginSuccess(access_token);
  browser->GetHost()->CloseBrowser(false);
  return true;
}
}  // namespace ncstreamer
