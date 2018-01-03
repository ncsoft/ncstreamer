/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#include "ncstreamer_cef/src/streaming_service/youtube.h"

#include <vector>

#include "boost/algorithm/string.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/property_tree/ptree.hpp"
#include "include/base/cef_bind.h"
#include "include/cef_browser.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

#include "ncstreamer_cef/src/lib/cef_types.h"
#include "ncstreamer_cef/src/lib/display.h"
#include "ncstreamer_cef/src/lib/windows_types.h"
#include "ncstreamer_cef/src/streaming_service/youtube_api.h"
#include "ncstreamer_cef/src/streaming_service/youtube_app_secret.h"


namespace ncstreamer {
YouTube::YouTube()
    : login_client_{},
      http_request_service_{},
      access_token_mutex_{},
      access_token_{},
      refresh_token_{} {
}


YouTube::~YouTube() {
}


static const char *kNcStreamerAppId{[]() {
  static const char *kProduction{
      "237911927015-d9or3i5c3mbc6f61oqe62mhgporb0r8a.apps.googleusercontent.com"};  // NOLINT
  return kProduction;
}()};


void YouTube::LogIn(
    HWND parent,
    const std::wstring &locale,
    const OnFailed &on_failed,
    const OnLoggedIn &on_logged_in) {
  static const Uri kLoginUri{YouTubeApi::Login::Oauth::Code::BuildUri(
      kNcStreamerAppId,
      YouTubeApi::Login::Redirect::static_uri(),
      {"https://www.googleapis.com/auth/youtube.force-ssl",
       "https://www.googleapis.com/auth/youtube.readonly"})};

  static const Dimension<int> kPopupDimension{429, 402};

  const Rectangle &parent_rect = Windows::GetWindowRectangle(parent);
  const Rectangle &popup_rect = parent_rect.Center(
      Display::Scale(kPopupDimension));

  CefWindowInfo window_info;
  window_info.SetAsPopup(parent, "YouTube Login");
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


void YouTube::LogOut(
    const OnFailed &on_failed,
    const OnLoggedOut &on_logged_out) {
}


void YouTube::PostLiveVideo(
    const std::string &stream_server,
    const std::string &user_page_id,
    const std::string &privacy,
    const std::string &title,
    const std::string &description,
    const std::string &app_attribution_tag,
    const OnFailed &on_failed,
    const OnLiveVideoPosted &on_live_video_posted) {
}


void YouTube::GetComments(const std::string &created_time,
    const OnFailed &on_failed,
    const OnCommentsGot &on_comments_got) {
}


void YouTube::GetLiveVideoViewers(
    const OnFailed &on_failed,
    const OnLiveVideoViewers &on_live_video_viewers) {
}


void YouTube::StopLiveVideo() {
}


void YouTube::GetRefreshToken() {
  Uri refresh_token_uri{
      YouTubeApi::Login::Oauth::RefreshToken::BuildUri()};

  Uri::Query post_content{{
      {"client_id", kNcStreamerAppId},
      {"client_secret", kYouTubeAppSecret},
      {"refresh_token", refresh_token_},
      {"grant_type", "refresh_token"}}};

  http_request_service_.Post(
      refresh_token_uri.uri_string(),
      post_content,
      [](const boost::system::error_code &ec) {
    // nothing to do.
  }, [this](const std::string &str) {
    boost::property_tree::ptree tree;
    std::stringstream ss{str};
    std::string access_token;
    int expires_in{0};
    try {
      boost::property_tree::read_json(ss, tree);
      access_token = tree.get<std::string>("access_token");
      expires_in = tree.get<int>("expires_in");  // second
    } catch (const std::exception &/*e*/) {
      access_token = "";
      expires_in = 0;
      return;
    }

    SetAccessToken(access_token);
    ::CefPostDelayedTask(
        TID_UI,
        base::Bind(&YouTube::GetRefreshToken,
                   base::Unretained(this)),
        (expires_in - 10) * 1000);
  });
}

void YouTube::GetChannel(
    const OnFailed &on_failed,
    const OnChannelGotten &on_channel_gotten) {
  Uri channel_uri{YouTubeApi::Graph::Channel::BuildUri(
      GetAccessToken())};

  http_request_service_.Get(
      channel_uri.uri_string(),
      [on_failed](const boost::system::error_code &ec) {
    std::string msg{ec.message()};
    on_failed(msg);
  }, [on_failed, on_channel_gotten](const std::string &str) {
    boost::property_tree::ptree tree;
    std::stringstream ss{str};
    std::vector<std::string> channels;
    try {
      boost::property_tree::read_json(ss, tree);
      const auto &items{tree.get_child("items")};
      if (items.size() < 1) {
        on_failed("no channel items");
        return;
      }
      for (const auto &channel : items) {
        const std::string &title{
            channel.second.get<std::string>("snippet.title")};
        channels.emplace_back(title);
      }
    } catch (const std::exception &/*e*/) {
      channels.clear();
    }

    if (channels.empty() == true) {
      std::stringstream msg;
      msg << "could not get channel from: " << str;
      on_failed(msg.str());
      return;
    }
    on_channel_gotten(channels[0]);
  });
}


void YouTube::OnLoginSuccess(
    const std::string &code,
    const OnFailed &on_failed,
    const OnLoggedIn &on_logged_in) {
  Uri access_token_uri{YouTubeApi::Login::Oauth::Token::BuildUri()};

  Uri::Query post_content{{
      {"code", code},
      {"client_id", kNcStreamerAppId},
      {"client_secret", kYouTubeAppSecret},
      {"redirect_uri", YouTubeApi::Login::Redirect::static_uri().uri_string()},
      {"grant_type", "authorization_code"}}};

  http_request_service_.Post(
      access_token_uri.uri_string(),
      post_content,
      [on_failed](const boost::system::error_code &ec) {
    std::string msg{ec.message()};
    on_failed(msg);
  }, [this, on_failed, on_logged_in](const std::string &str) {
    boost::property_tree::ptree tree;
    std::stringstream ss{str};
    std::string access_token;
    int expires_in;
    try {
      boost::property_tree::read_json(ss, tree);
      access_token = tree.get<std::string>("access_token");
      refresh_token_ = tree.get<std::string>("refresh_token");
      expires_in = tree.get<int>("expires_in");
    } catch (const std::exception &/*e*/) {
      access_token = "";
      refresh_token_ = "";
      expires_in = 0;
    }

    if (access_token.empty() == true) {
      std::stringstream msg;
      msg << "could not get token from: " << str;
      on_failed(msg.str());
      return;
    }

    SetAccessToken(access_token);
    ::CefPostDelayedTask(
        TID_UI,
        base::Bind(&YouTube::GetRefreshToken,
                   base::Unretained(this)),
        (expires_in - 10) * 1000);

    GetChannel(on_failed, [on_logged_in](
        const std::string &user_name) {
      on_logged_in(user_name, {}, {});
    });
  });
}


std::string YouTube::GetAccessToken() const {
  std::string access_token{};
  {
    std::lock_guard<std::mutex> lock{access_token_mutex_};
    access_token = access_token_;
  }
  return access_token;
}


void YouTube::SetAccessToken(const std::string &access_token) {
  std::lock_guard<std::mutex> lock{access_token_mutex_};
  access_token_ = access_token;
}


YouTube::LoginClient::LoginClient(
    YouTube *const owner,
    const HWND &base_window,
    const OnFailed &on_failed,
    const OnLoggedIn &on_logged_in)
    : owner_{owner},
      base_window_{base_window},
      on_failed_{on_failed},
      on_logged_in_{on_logged_in} {
}


YouTube::LoginClient::~LoginClient() {
}


CefRefPtr<CefLifeSpanHandler>
    YouTube::LoginClient::GetLifeSpanHandler() {
  return this;
}


CefRefPtr<CefRequestHandler>
    YouTube::LoginClient::GetRequestHandler() {
  return this;
}


void YouTube::LoginClient::OnAfterCreated(
    CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  HWND wnd = browser->GetHost()->GetWindowHandle();

  HICON icon = (HICON) ::SendMessage(
      base_window_, WM_GETICON, ICON_SMALL, (LPARAM)NULL);
  ::SendMessage(wnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
}


bool YouTube::LoginClient::OnBeforeBrowse(
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
      YouTubeApi::Login::Redirect::static_uri().scheme_authority_path()) {
    return OnLoginRedirected(browser, frame, request, is_redirect, uri);
  }

  return false;  // proceed navigation.
}


bool YouTube::LoginClient::OnLoginRedirected(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> /*frame*/,
    CefRefPtr<CefRequest> /*request*/,
    bool /*is_redirect*/,
    const Uri &uri) {
  CEF_REQUIRE_UI_THREAD();

  std::string &code =
      YouTubeApi::Login::Redirect::ExtractAccessCode(
          Uri::Query{uri.query()});

  if (code.empty() == true) {
    // login canceled.
  } else {
    owner_->OnLoginSuccess(code, on_failed_, on_logged_in_);
  }

  browser->GetHost()->CloseBrowser(false);
  return true;
}
}  // namespace ncstreamer
