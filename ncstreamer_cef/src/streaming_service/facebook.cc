/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/streaming_service/facebook.h"

#include <cassert>
#include <functional>
#include <regex>  // NOLINT
#include <string>
#include <unordered_map>

#include "boost/property_tree/json_parser.hpp"
#include "boost/property_tree/ptree.hpp"
#include "include/base/cef_bind.h"
#include "include/cef_cookie.h"
#include "include/cef_browser.h"
#include "include/cef_parser.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

#include "ncstreamer_cef/src/lib/cef_types.h"
#include "ncstreamer_cef/src/lib/display.h"
#include "ncstreamer_cef/src/lib/http_types.h"
#include "ncstreamer_cef/src/lib/uri.h"
#include "ncstreamer_cef/src/lib/windows_types.h"
#include "ncstreamer_cef/src/streaming_service/facebook_api.h"


namespace ncstreamer {
Facebook::Facebook()
    : login_client_{},
      http_request_service_{},
      access_token_mutex_{},
      access_token_{},
      me_info_mutex_{},
      me_info_{} {
}


Facebook::~Facebook() {
}


void Facebook::LogIn(
    HWND parent,
    const std::wstring &locale,
    const OnFailed &on_failed,
    const OnLoggedIn &on_logged_in) {
  static const char *kNcStreamerAppId{[]() {
    static const char *kProduction{
        "1789696898019802"};
    static const char *kAlpha{
        "1795379417451550"};
    return kProduction;
  }()};

  static const Uri kLoginUri{FacebookApi::Login::Oauth::BuildUri(
      kNcStreamerAppId,
      FacebookApi::Login::Redirect::static_uri(),
      "token",
      "popup",
      {"pages_show_list",
       "publish_video",
       "publish_pages",
       "manage_pages"})};

  static const Dimension<int> kPopupDimension{429, 402};

  const Rectangle &parent_rect = Windows::GetWindowRectangle(parent);
  const Rectangle &popup_rect = parent_rect.Center(
      Display::Scale(kPopupDimension));

  CefWindowInfo window_info;
  window_info.SetAsPopup(parent, "Facebook Login");
  CefWindowRectangle::Reset(popup_rect, &window_info);

  CefBrowserSettings browser_settings;
  CefString(&browser_settings.accept_language_list) = locale;

  login_client_ = new LoginClient{
      this, parent, on_failed, on_logged_in};

  CefBrowserHost::CreateBrowser(
      window_info,
      login_client_,
      kLoginUri.uri_string(),
      browser_settings,
      NULL);
}


void Facebook::LogOut(
    const OnFailed &on_failed,
    const OnLoggedOut &on_logged_out) {
  if (::CefCurrentlyOn(TID_IO) == false) {
    ::CefPostTask(TID_IO,
        base::Bind(&Facebook::LogOut, base::Unretained(this),
            on_failed, on_logged_out));
    return;
  }

  class LogoutCallback : public CefDeleteCookiesCallback {
   public:
    LogoutCallback(
        Facebook *caller,
        const OnFailed &on_failed,
        const OnLoggedOut &on_logged_out)
        : CefDeleteCookiesCallback{},
          caller_{caller},
          on_failed_{on_failed},
          on_logged_out_{on_logged_out} {}

    virtual ~LogoutCallback() {}

    void OnComplete(int /*num_deleted*/) override {
      caller_->SetAccessToken("");
      caller_->SetMeInfo({"", "", {}});
      caller_->SetStreamId("");
      on_logged_out_();
    }

   private:
    Facebook *caller_;
    OnFailed on_failed_;
    OnLoggedOut on_logged_out_;

    IMPLEMENT_REFCOUNTING(LogoutCallback);
  };

  CefRefPtr<LogoutCallback> logout_callback{
      new LogoutCallback{this, on_failed, on_logged_out}};
  CefCookieManager::GetGlobalManager(NULL)->DeleteCookies(
      L"",
      L"",
      logout_callback);
}


void Facebook::PostLiveVideo(
    const std::string &stream_server,
    const std::string &user_page_id,
    const std::string &privacy,
    const std::string &title,
    const std::string &description,
    const std::string &app_attribution_tag,
    const OnFailed &on_failed,
    const OnLiveVideoPosted &on_live_video_posted) {
  if (user_page_id.empty() == true ||
      privacy.empty() == true) {
    assert(false);
    return;
  }

  Uri live_video_uri{FacebookApi::Graph::LiveVideos::BuildUri(
      user_page_id,
      app_attribution_tag)};

  const std::string &access_token = (user_page_id == "me") ?
      GetAccessToken() : GetPageAccessToken(user_page_id);
  if (access_token.empty() == true) {
    std::stringstream msg;
    msg << "invalid user page: " << user_page_id;
    on_failed(msg.str());
    return;
  }

  boost::property_tree::ptree post_content{
      FacebookApi::Graph::LiveVideos::BuildPostContent(
          access_token,
          privacy,
          title,
          description)};

  http_request_service_.Post(
      live_video_uri.uri_string(),
      post_content,
      [on_failed](const boost::system::error_code &ec) {
    std::string msg{ec.message()};
    on_failed(msg);
  }, [this, on_failed, on_live_video_posted](
      const std::string &str) {
    boost::property_tree::ptree tree;
    std::stringstream ss{str};
    std::string stream_id{};
    std::string stream_url{};
    try {
      boost::property_tree::read_json(ss, tree);
      stream_id = tree.get<std::string>("id");
      stream_url = tree.get<std::string>("stream_url");
    } catch (const std::exception &/*e*/) {
      stream_url = "";
    }

    SetStreamId(stream_id);
    if (stream_url.empty() == true) {
      std::stringstream msg;
      msg << "could not get stream_url from: " << str;
      on_failed(msg.str());
      return;
    }

    OutputDebugStringA((stream_url + "/stream_url\r\n").c_str());

    GetPostUrl(
        stream_id, stream_url, on_failed, on_live_video_posted);
  });
}


void Facebook::GetComments(const std::string &created_time,
    const OnFailed &on_failed,
    const OnCommentsGot &on_comments_got) {
  Uri live_video_comments_uri{FacebookApi::Graph::LiveVideoComments::BuildUri(
      GetAccessToken(), GetStreamId(), created_time)};

  OutputDebugStringA(live_video_comments_uri.uri_string().c_str());
  http_request_service_.Get(
      live_video_comments_uri.uri_string(),
      [on_failed](const boost::system::error_code &ec) {
        std::string msg{ec.message()};
        on_failed(msg);
      },
      [on_comments_got](const std::string &str) {
        OutputDebugStringA(str.c_str());
        on_comments_got(str);
  });
}


void Facebook::GetLiveVideoViewers(
    const OnFailed &on_failed,
    const OnLiveVideoViewers &on_live_video_viewers) {
  Uri live_video_info_uri{FacebookApi::Graph::LiveVideoInfo::BuildUri(
      GetAccessToken(), GetStreamId(), {"live_views"})};

  OutputDebugStringA(live_video_info_uri.uri_string().c_str());
  http_request_service_.Get(
      live_video_info_uri.uri_string(),
      [on_failed](const boost::system::error_code &ec) {
    std::string msg{ec.message()};
    on_failed(msg);
  }, [on_live_video_viewers](const std::string &str) {
    OutputDebugStringA(str.c_str());
    on_live_video_viewers(str);
  });
}


void Facebook::StopLiveVideo() {
  SetStreamId("");
}


std::vector<StreamingServiceProvider::UserPage>
    Facebook::ExtractAccountAll(
        const boost::property_tree::ptree &tree) {
  std::vector<UserPage> accounts;

  const auto &arr = tree.get_child("data");
  for (const auto &elem : arr) {
    const auto &account = elem.second;
    const auto &id = account.get<std::string>("id");
    const auto &name = account.get<std::string>("name");
    const auto &link = account.get<std::string>("link");
    const auto &access_token = account.get<std::string>("access_token");
    accounts.emplace_back(id, name, link, access_token);
  }

  return accounts;
}


void Facebook::GetMe(
    const OnFailed &on_failed,
    const OnMeGotten &on_me_gotten) {
  Uri me_uri{FacebookApi::Graph::Me::BuildUri(
      GetAccessToken(),
      {"id",
       "name",
       "link",
       "accounts{id,name,link,access_token}"})};

  http_request_service_.Get(
      me_uri.uri_string(),
      [on_failed](const boost::system::error_code &ec) {
    std::string msg{ec.message()};
    on_failed(msg);
  }, [on_failed, on_me_gotten](const std::string &str) {
    boost::property_tree::ptree me;
    std::stringstream me_ss{str};
    std::string me_id{};
    std::string me_name{};
    try {
      boost::property_tree::read_json(me_ss, me);
      me_id = me.get<std::string>("id");
      me_name = me.get<std::string>("name");
    } catch (const std::exception &/*e*/) {
      me_id = "";
      me_name = "";
    }

    if (me_id.empty() == true) {
      std::stringstream msg;
      msg << "could not get me from: " << str;
      on_failed(msg.str());
      return;
    }

    std::vector<UserPage> me_accounts;
    try {
      me_accounts = ExtractAccountAll(me.get_child("accounts"));
    } catch (...) {
    }

    on_me_gotten(me_id, me_name, me_accounts);
  });
}


void Facebook::GetPostUrl(
    const std::string &stream_id,
    const std::string &stream_url,
    const OnFailed &on_failed,
    const OnLiveVideoPosted &on_live_video_posted) {
  Uri post_id_uri{FacebookApi::Graph::PostId::BuildUri(
      GetAccessToken(), stream_id)};

  http_request_service_.Get(
      post_id_uri.uri_string(),
      [on_failed](const boost::system::error_code &ec) {
    std::string msg{ec.message()};
    on_failed(msg);
  }, [stream_id, stream_url, on_failed, on_live_video_posted](
      const std::string &str) {
    boost::property_tree::ptree stream;
    std::stringstream stream_ss{str};
    std::string post_url{};
    try {
      boost::property_tree::read_json(stream_ss, stream);
      std::string embed_html = stream.get<std::string>("embed_html");
      std::smatch matches;
      static const std::regex kUriPattern{R"(href=(.*?)(?:\&|\"))"};
      bool found = std::regex_search(embed_html, matches, kUriPattern);
      if (found) {
        std::string encoded_url{matches[1]};
        cef_uri_unescape_rule_t escape_rule =
            static_cast<cef_uri_unescape_rule_t>(UU_PATH_SEPARATORS |
                UU_URL_SPECIAL_CHARS_EXCEPT_PATH_SEPARATORS);
        post_url = ::CefURIDecode(encoded_url, true, escape_rule);
      }
    } catch (const std::exception &/*e*/) {
      post_url.empty();
    }

    if (post_url.empty() == true) {
      std::stringstream msg;
      msg << "could not get post_url from: " << str;
      on_failed(msg.str());
      return;
    }
    std::string stream_server;
    std::string stream_key;
    std::tie(stream_server, stream_key) = [stream_url]() {
      std::size_t key_index = stream_url.find_last_of('/') + 1;
      return std::make_tuple(stream_url.substr(0, key_index),
          stream_url.substr(key_index));
    }();
    on_live_video_posted(
        stream_server, stream_key, stream_id, post_url);
  });
}


void Facebook::OnLoginSuccess(
    const std::string &access_token,
    const OnFailed &on_failed,
    const OnLoggedIn &on_logged_in) {
  SetAccessToken(access_token);

  GetMe(on_failed, [this, access_token, on_logged_in](
      const std::string &me_id,
      const std::string &me_name,
      const std::vector<UserPage> &me_accounts) {
    AccountMap me_accounts_as_map{};
    for (const auto &account : me_accounts) {
      me_accounts_as_map.emplace(account.id(), account);
    }

    SetMeInfo({
        me_id, me_name, me_accounts_as_map});

    OutputDebugStringA((me_id + "/id\r\n").c_str());
    OutputDebugStringA((me_name + "/name\r\n").c_str());
    OutputDebugStringA(
        (std::to_string(me_accounts.size()) + "/accounts\r\n").c_str());

    on_logged_in(me_id, access_token, me_name, me_accounts, {});
  });
}


std::string Facebook::GetAccessToken() const {
  std::string access_token{};
  {
    std::lock_guard<std::mutex> lock{access_token_mutex_};
    access_token = access_token_;
  }
  return access_token;
}


std::string Facebook::GetPageAccessToken(const std::string &page_id) const {
  std::string page_access_token{};

  {
    std::lock_guard<std::mutex> lock{me_info_mutex_};

    const auto &me_accounts = std::get<2>(me_info_);;
    auto i = me_accounts.find(page_id);
    if (i != me_accounts.end()) {
      page_access_token = i->second.access_token();
    }
  }

  return page_access_token;
}


std::string Facebook::GetStreamId() const {
  std::string stream_id{};
  {
    std::lock_guard<std::mutex> lock{stream_id_mutex_};
    stream_id = stream_id_;
  }
  return stream_id;
}


void Facebook::SetAccessToken(const std::string &access_token) {
  std::lock_guard<std::mutex> lock{access_token_mutex_};
  access_token_ = access_token;
}


void Facebook::SetMeInfo(const MeInfo &me_info) {
  std::lock_guard<std::mutex> lock{me_info_mutex_};
  me_info_ = me_info;
}


void Facebook::SetStreamId(const std::string &stream_id) {
  std::lock_guard<std::mutex> lock{stream_id_mutex_};
  stream_id_ = stream_id;
}

Facebook::LoginClient::LoginClient(
    Facebook *const owner,
    const HWND &base_window,
    const OnFailed &on_failed,
    const OnLoggedIn &on_logged_in)
    : owner_{owner},
      base_window_{base_window},
      on_failed_{on_failed},
      on_logged_in_{on_logged_in} {
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

  Uri uri{request->GetURL()};
  OutputDebugStringA((uri.uri_string() + "\r\n").c_str());

  if (uri.scheme_authority_path() ==
      FacebookApi::Login::Redirect::static_uri().scheme_authority_path()) {
    return OnLoginRedirected(browser, frame, request, is_redirect, uri);
  }

  return false;  // proceed navigation.
}


bool Facebook::LoginClient::OnLoginRedirected(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> /*frame*/,
    CefRefPtr<CefRequest> /*request*/,
    bool /*is_redirect*/,
    const Uri &uri) {
  CEF_REQUIRE_UI_THREAD();

  const std::string &access_token =
      FacebookApi::Login::Redirect::ExtractAccessToken(
          Uri::Query{uri.fragment()});

  if (access_token.empty() == true) {
    // login canceled.
  } else {
    owner_->OnLoginSuccess(
        access_token, on_failed_, on_logged_in_);
  }

  browser->GetHost()->CloseBrowser(false);
  return true;
}
}  // namespace ncstreamer
