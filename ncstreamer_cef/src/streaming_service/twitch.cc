/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#include "ncstreamer_cef/src/streaming_service/twitch.h"

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
#include "ncstreamer_cef/src/streaming_service/twitch_api.h"


namespace ncstreamer {
Twitch::Twitch()
    : login_client_{},
      http_request_service_{},
      access_token_mutex_{},
      access_token_{},
      nick_name_mutex_{},
      nick_name_{},
      account_name_mutex_{},
      account_name_{},
      channel_id_mutex_{},
      channel_id_{} {
}


Twitch::~Twitch() {
}


static const char *kNcStreamerAppId{[]() {
  static const char *kProduction{
      "c1q480zb66arf4rgaji54nh4bpn86o"};
  return kProduction;
}()};


static const char *kTwitchIrcHost{ []() {
  static const char *kSSLHost{
      "irc.chat.twitch.tv"};
  return kSSLHost;
}()};


static const char *kTwitchIrcPort{ []() {
  static const char *kSSLPort{
      "6697"};
  return kSSLPort;
}()};


void Twitch::LogIn(
    HWND parent,
    const std::wstring &locale,
    const OnFailed &on_failed,
    const OnLoggedIn &on_logged_in) {
  static const Uri kLoginUri{TwitchApi::Login::Oauth::Token::BuildUri(
      kNcStreamerAppId,
      TwitchApi::Login::Redirect::static_uri(),
      {"user_read", "channel_read", "channel_editor", "chat_login"})};

  static const Dimension<int> kPopupDimension{429, 402};

  const Rectangle &parent_rect = Windows::GetWindowRectangle(parent);
  const Rectangle &popup_rect = parent_rect.Center(
      Display::Scale(kPopupDimension));

  CefWindowInfo window_info;
  window_info.SetAsPopup(parent, "Twitch Login");
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


void Twitch::LogOut(
    const OnFailed &on_failed,
    const OnLoggedOut &on_logged_out) {
  if (::CefCurrentlyOn(TID_IO) == false) {
    ::CefPostTask(TID_IO,
        base::Bind(&Twitch::LogOut, base::Unretained(this),
            on_failed, on_logged_out));
    return;
  }

  class LogoutCallback : public CefDeleteCookiesCallback {
   public:
    LogoutCallback(
        Twitch *caller,
        const OnFailed &on_failed,
        const OnLoggedOut &on_logged_out)
        : CefDeleteCookiesCallback{},
          caller_{caller},
          on_failed_{on_failed},
          on_logged_out_{on_logged_out} {}

    virtual ~LogoutCallback() {}

    void OnComplete(int /*num_deleted*/) override {
      caller_->SetAccessToken("");
      caller_->SetNickName("");
      caller_->SetAccountName("");
      caller_->SetChannelId("");
      on_logged_out_();
    }

   private:
    Twitch *caller_;
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


void Twitch::PostLiveVideo(
    const std::string &stream_server,
    const std::string &user_page_id,
    const std::string &privacy,
    const std::string &title,
    const std::string &description,
    const std::string &app_attribution_tag,
    const OnFailed &on_failed,
    const OnLiveVideoPosted &on_live_video_posted) {
  if (stream_server.empty() == true) {
    assert(false);
    return;
  }
  GetChannel(on_failed, [this, stream_server, description, on_failed,
                         on_live_video_posted](
      const std::string &channel_id,
      const std::string &post_url,
      const std::string &stream_key) {
    UpdateChannel(channel_id,
                  description,
                  "",
                  stream_server,
                  stream_key,
                  post_url,
                  on_failed,
                  on_live_video_posted);
  });

  chat_.Connect(kTwitchIrcHost, kTwitchIrcPort, GetAccessToken(),
      GetNickName(), GetAccountNameLowerCase(),
      [](const boost::system::error_code &/*ec*/) {});
}


void Twitch::GetComments(const std::string &created_time,
    const OnFailed &on_failed,
    const OnCommentsGot &on_comments_got) {
  switch (chat_.GetReadyStatus()) {
  case IrcService::ReadyStatus::kNone:
  case IrcService::ReadyStatus::kConnecting:
    on_failed("not ready");
    break;
  case IrcService::ReadyStatus::kCompleted:
    on_comments_got(chat_.GetJson(created_time));
    break;
  default:
    assert(false);
    break;
  }
}


void Twitch::GetLiveVideoViewers(
    const OnFailed &on_failed,
    const OnLiveVideoViewers &on_live_video_viewers) {
  using boost::property_tree::json_parser::write_json;

  Uri live_stream_uri{TwitchApi::Graph::LiveStream::BuildUri(
      kNcStreamerAppId,
      GetChannelId(),
      GetAccessToken())};

  http_request_service_.Get(
      live_stream_uri.uri_string(),
      [on_failed](const boost::system::error_code &ec) {
    std::string msg{ec.message()};
    on_failed(msg);
  }, [on_live_video_viewers](const std::string &str) {
    OutputDebugStringA(str.c_str());

    std::string cnt = "0";
    try {
      boost::property_tree::ptree root;
      std::stringstream root_ss{str};
      boost::property_tree::read_json(root_ss, root);

      const auto &stream = root.get<std::string>("stream");
      if (stream != "null") {
        const auto &child = root.get_child("stream", {});
        cnt = child.get<std::string>("viewers");
      }
    } catch (const std::exception &/*e*/) {
      cnt = "0";
    }

    boost::property_tree::ptree wroot;
    wroot.add<std::string>("live_views", cnt);
    std::ostringstream oss;
    write_json(oss, wroot);
    on_live_video_viewers(oss.str());
  });
}


void Twitch::StopLiveVideo() {
  chat_.Close();
}


void Twitch::GetUser(
    const OnFailed &on_failed,
    const OnUserGotten &on_user_gotten) {
  Uri user_uri{TwitchApi::Graph::User::BuildUri(
      kNcStreamerAppId,
      GetAccessToken())};

  http_request_service_.Get(
      user_uri.uri_string(),
      [on_failed](const boost::system::error_code &ec) {
    std::string msg{ec.message()};
    on_failed(msg);
  }, [this, on_failed, on_user_gotten](const std::string &str) {
    boost::property_tree::ptree tree;
    std::stringstream ss{str};
    std::string nick_name{};
    std::string account_name{};
    try {
      boost::property_tree::read_json(ss, tree);
      nick_name = tree.get<std::string>("display_name");
      account_name = tree.get<std::string>("name");
    } catch (const std::exception &/*e*/) {
      nick_name = "";
      account_name = "";
    }
    SetNickName(nick_name);
    SetAccountName(account_name);

    if ((nick_name.empty() == true) || (account_name.empty() == true)) {
      std::stringstream msg;
      msg << "could not get user from: " << str;
      on_failed(msg.str());
      return;
    }
    const std::string &user_page{
        "https://www.twitch.tv/" +
        account_name +
        "/dashboard/settings"};
    on_user_gotten(nick_name, user_page);
  });
}


void Twitch::GetChannel(
    const OnFailed &on_failed,
    const OnChannelGotten &on_channel_gotten) {
  Uri channel_uri{TwitchApi::Graph::Channel::BuildUri(
      kNcStreamerAppId,
      GetAccessToken())};

  http_request_service_.Get(
      channel_uri.uri_string(),
      [on_failed](const boost::system::error_code &ec) {
    std::string msg{ec.message()};
    on_failed(msg);
  }, [this, on_failed, on_channel_gotten](const std::string &str) {
    boost::property_tree::ptree tree;
    std::stringstream ss{str};
    std::string channel_id;
    std::string post_url{};
    std::string stream_key{};
    try {
      boost::property_tree::read_json(ss, tree);
      channel_id = tree.get<std::string>("_id");
      post_url = tree.get<std::string>("url");
      stream_key = tree.get<std::string>("stream_key");
    } catch (const std::exception &/*e*/) {
      channel_id = "";
      post_url = "";
      stream_key = "";
    }

    if (channel_id.empty() == true) {
      std::stringstream msg;
      msg << "could not get channel from: " << str;
      on_failed(msg.str());
      return;
    }

    SetChannelId(channel_id);
    on_channel_gotten(channel_id, post_url, stream_key);
  });
}


void Twitch::UpdateChannel(
    const std::string &channel_id,
    const std::string &description,
    const std::string &game,
    const std::string &stream_server,
    const std::string &stream_key,
    const std::string &post_url,
    const OnFailed &on_failed,
    const OnLiveVideoPosted &on_live_video_posted) {
  Uri update_channel_uri{TwitchApi::Graph::UpdateChannel::BuildUri(
      channel_id,
      GetAccessToken())};

  boost::property_tree::ptree post_content{
      TwitchApi::Graph::UpdateChannel::BuildPostContent(
          description,
          game,
          true)};

  http_request_service_.Put(
      update_channel_uri.uri_string(),
      post_content,
      [on_failed](const boost::system::error_code &ec) {
    std::string msg{ec.message()};
    on_failed(msg);
  }, [stream_server,
      stream_key,
      post_url,
      on_live_video_posted](const std::string &str) {
    on_live_video_posted(stream_server, stream_key, post_url);
  });
}


void Twitch::GetStreamServers(
    const OnFailed &on_failed,
    const OnServerListGotten &on_server_list_gotten) {
  Uri get_server_list_uri{TwitchApi::Graph::Ingests::BuildUri(
      kNcStreamerAppId)};

  http_request_service_.Get(
      get_server_list_uri.uri_string(),
      [on_failed](const boost::system::error_code &ec) {
    std::string msg{ec.message()};
    on_failed(msg);
  }, [on_failed, on_server_list_gotten](const std::string &str) {
    boost::property_tree::ptree tree;
    std::stringstream ss{str};
    std::vector<StreamServer> stream_servers{};
    try {
      boost::property_tree::read_json(ss, tree);
      const auto &list = tree.get_child("ingests");
      for (const auto &elem : list) {
        const auto &server = elem.second;
        const auto &id = server.get<std::string>("_id");
        const auto &name = server.get<std::string>("name");
        const auto &availability = server.get<std::string>("availability");
        const auto &url_template = server.get<std::string>("url_template");
        const std::string &url =
            url_template.substr(0, url_template.find_last_of("/"));
        stream_servers.emplace_back(id, name, url, availability);
      }
    } catch (const std::exception &/*e*/) {
      stream_servers.clear();
    }

    if (stream_servers.empty() == true) {
      std::stringstream msg;
      msg << "could not get stream servers from: " << str;
      on_failed(msg.str());
      return;
    }
    on_server_list_gotten(stream_servers);
  });
}


void Twitch::OnLoginSuccess(
    const std::string &access_token,
    const OnFailed &on_failed,
    const OnLoggedIn &on_logged_in) {
  SetAccessToken(access_token);

  GetUser(on_failed, [this, on_failed, on_logged_in](
      const std::string &name,
      const std::string &user_page) {
    GetStreamServers(on_failed, [name, user_page, on_logged_in](
        const std::vector<StreamServer> &stream_servers) {
      StreamingServiceProvider::UserPage page{"", "", user_page, ""};
      on_logged_in(name, {page}, stream_servers);
    });
  });
}


std::string Twitch::GetAccessToken() const {
  std::string access_token{};
  {
    std::lock_guard<std::mutex> lock{access_token_mutex_};
    access_token = access_token_;
  }
  return access_token;
}


void Twitch::SetAccessToken(const std::string &access_token) {
  std::lock_guard<std::mutex> lock{access_token_mutex_};
  access_token_ = access_token;
}


std::string Twitch::GetNickName() const {
  std::string nick_name{};
  {
    std::lock_guard<std::mutex> lock{nick_name_mutex_};
    nick_name = nick_name_;
  }
  return nick_name;
}


void Twitch::SetNickName(const std::string &nick_name) {
  std::lock_guard<std::mutex> lock{nick_name_mutex_};
  nick_name_ = nick_name;
}


std::string Twitch::GetAccountNameLowerCase() const {
  std::string account_name{};
  {
    std::lock_guard<std::mutex> lock{account_name_mutex_};
    account_name = account_name_;
  }
  boost::algorithm::to_lower(account_name);
  return account_name;
}


void Twitch::SetAccountName(const std::string &account_name) {
  std::lock_guard<std::mutex> lock{account_name_mutex_};
  account_name_ = account_name;
}


std::string Twitch::GetChannelId() const {
  std::string channel_id{};
  {
    std::lock_guard<std::mutex> lock{channel_id_mutex_};
    channel_id = channel_id_;
  }
  return channel_id;
}


void Twitch::SetChannelId(const std::string &channel_id) {
  std::lock_guard<std::mutex> lock{channel_id_mutex_};
  channel_id_ = channel_id;
}


Twitch::LoginClient::LoginClient(
    Twitch *const owner,
    const HWND &base_window,
    const OnFailed &on_failed,
    const OnLoggedIn &on_logged_in)
    : owner_{owner},
      base_window_{base_window},
      on_failed_{on_failed},
      on_logged_in_{on_logged_in} {
}


Twitch::LoginClient::~LoginClient() {
}


CefRefPtr<CefLifeSpanHandler>
    Twitch::LoginClient::GetLifeSpanHandler() {
  return this;
}


CefRefPtr<CefRequestHandler>
    Twitch::LoginClient::GetRequestHandler() {
  return this;
}


void Twitch::LoginClient::OnAfterCreated(
    CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  HWND wnd = browser->GetHost()->GetWindowHandle();

  HICON icon = (HICON) ::SendMessage(
      base_window_, WM_GETICON, ICON_SMALL, (LPARAM)NULL);
  ::SendMessage(wnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
}


bool Twitch::LoginClient::OnBeforeBrowse(
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
    TwitchApi::Login::Redirect::static_uri().scheme_authority_path()) {
    return OnLoginRedirected(browser, frame, request, is_redirect, uri);
  }

  return false;  // proceed navigation.
}


bool Twitch::LoginClient::OnLoginRedirected(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> /*frame*/,
    CefRefPtr<CefRequest> /*request*/,
    bool /*is_redirect*/,
    const Uri &uri) {
  CEF_REQUIRE_UI_THREAD();

  const std::string &access_token =
      TwitchApi::Login::Redirect::ExtractAccessToken(
          Uri::Query{uri.fragment()});

  if (access_token.empty() == true) {
    // login canceled.
  } else {
    owner_->OnLoginSuccess(access_token, on_failed_, on_logged_in_);
  }

  browser->GetHost()->CloseBrowser(false);
  return true;
}
}  // namespace ncstreamer
