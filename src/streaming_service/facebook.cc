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

#ifdef _MSC_VER
#pragma warning(disable: 4819)
#endif
#include "boost/property_tree/json_parser.hpp"
#ifdef _MSC_VER
#pragma warning(default: 4819)
#endif

#include "boost/property_tree/ptree.hpp"
#include "include/cef_browser.h"
#include "include/wrapper/cef_helpers.h"

#include "src/lib/cef_types.h"
#include "src/lib/uri.h"
#include "src/lib/windows_types.h"
#include "src/streaming_service/facebook_api.h"


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

  static const Uri kLoginUri{FacebookApi::Login::Oauth::BuildUri(
      kNcStreamerAppId,
      FacebookApi::Login::Redirect::static_uri(),
      L"token",
      L"popup",
      {L"pages_show_list"})};

  const Rectangle &parent_rect = Windows::GetWindowRectangle(parent);
  const Rectangle &popup_rect = parent_rect.Center(429, 402);

  CefWindowInfo window_info;
  window_info.SetAsPopup(parent, L"Facebook Login");
  CefWindowRectangle::Reset(popup_rect, &window_info);

  CefBrowserSettings browser_settings;

  facebook_client_->SetHandlers(on_failed, on_logged_in);
  CefBrowserHost::CreateBrowser(
      window_info,
      facebook_client_,
      kLoginUri.uri_string(),
      browser_settings,
      NULL);
}


Facebook::FacebookClient::FacebookClient()
    : access_token_{},
      me_id_{},
      me_name_{},
      me_accounts_{},
      on_failed_{},
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


CefRefPtr<CefLoadHandler>
    Facebook::FacebookClient::GetLoadHandler() {
  return this;
}


CefRefPtr<CefRequestHandler>
    Facebook::FacebookClient::GetRequestHandler() {
  return this;
}


void Facebook::FacebookClient::OnLoadEnd(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    int http_status_code) {
  CEF_REQUIRE_UI_THREAD();

  if (frame->IsMain() == false) {
    return;
  }

  using Handler = std::function<void(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      int http_status_code,
      const Uri &uri)>;

  using Api = FacebookApi;
  using This = Facebook::FacebookClient;

  static const std::unordered_map<std::wstring, Handler> kHandlers{
      {Api::Graph::Me::static_uri().scheme_authority_path(),
       std::bind(&This::OnGetMe, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3,
           std::placeholders::_4)}};

  Uri uri{frame->GetURL()};
  OutputDebugString((uri.uri_string() + L"\r\n").c_str());

  auto i = kHandlers.find(uri.scheme_authority_path());
  if (i == kHandlers.end()) {
    return;
  }

  i->second(browser, frame, http_status_code, uri);
}


bool Facebook::FacebookClient::OnBeforeBrowse(
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
  using This = Facebook::FacebookClient;

  static const std::unordered_map<std::wstring, Handler> kHandlers{
      {Api::Login::Redirect::static_uri().scheme_authority_path(),
       std::bind(&This::OnAccessToken, this,
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


std::vector<StreamingServiceProvider::UserPage>
    Facebook::FacebookClient::ExtractAccountAll(
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


void Facebook::FacebookClient::GetMe(
    const CefRefPtr<CefFrame> &frame,
    const std::wstring &access_token) {
  Uri me_uri{FacebookApi::Graph::Me::BuildUri(
      access_token,
      {L"id",
       L"name",
       L"accounts"})};
  frame->LoadURL(me_uri.uri_string());
}


void Facebook::FacebookClient::OnGetMe(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    int /*http_status_code*/,
    const Uri &uri) {
  CEF_REQUIRE_UI_THREAD();

  class Visitor : public CefStringVisitor {
   public:
    using OnVisit = std::function<void(const std::wstring &str)>;
    explicit Visitor(const OnVisit &on_visit) : on_visit_{on_visit} {}
   protected:
    void Visit(const CefString &str) override { on_visit_(str); }
   private:
    OnVisit on_visit_;
    IMPLEMENT_REFCOUNTING(Visitor);
  };

  CefRefPtr<Visitor> visitor{new Visitor{[this, browser](
      const std::wstring &str) {
    CEF_REQUIRE_UI_THREAD();

    OutputDebugString((str + L"\r\n").c_str());

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::string utf8 = converter.to_bytes(str);

    boost::property_tree::ptree me;
    std::stringstream me_ss{utf8};
    std::wstring id{};
    std::wstring name{};
    std::vector<UserPage> accounts;
    try {
      boost::property_tree::read_json(me_ss, me);
      id = converter.from_bytes(me.get<std::string>("id"));
      name = converter.from_bytes(me.get<std::string>("name"));
      accounts = ExtractAccountAll(me.get_child("accounts"));
    } catch (const std::exception &/*e*/) {
    }

    if (id.empty() == true) {
      std::wstringstream msg;
      msg << L"could not get me from: " << str;
      on_failed_(msg.str());
      return;
    }

    me_id_ = id;
    me_name_ = name;
    for (const auto &account : accounts) {
      me_accounts_.emplace(account.id(), account);
    }

    OutputDebugString((me_id_ + L"/id\r\n").c_str());
    OutputDebugString((me_name_ + L"/name\r\n").c_str());
    OutputDebugString(
        (std::to_wstring(me_accounts_.size()) + L"/accounts\r\n").c_str());

    on_logged_in_(name, accounts);

    browser->GetHost()->CloseBrowser(false);
  }}};

  frame->GetText(visitor);
}


bool Facebook::FacebookClient::OnAccessToken(
    CefRefPtr<CefBrowser> /*browser*/,
    CefRefPtr<CefFrame> frame,
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

  access_token_ = access_token;
  GetMe(frame, access_token);
  return true;
}
}  // namespace ncstreamer
