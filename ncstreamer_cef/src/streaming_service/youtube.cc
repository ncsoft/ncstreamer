/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#include "ncstreamer_cef/src/streaming_service/youtube.h"

#include <deque>
#include <regex>  // NOLINT
#include <tuple>
#include <unordered_map>
#include <utility>
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
      refresh_token_{},
      video_id_mutex_{},
      video_id_{},
      live_chat_id_mutex_{},
      live_chat_id_{} {
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
       "https://www.googleapis.com/auth/youtube.readonly",
       "https://www.googleapis.com/auth/youtubepartner"})};

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
  if (::CefCurrentlyOn(TID_IO) == false) {
    ::CefPostTask(TID_IO,
        base::Bind(&YouTube::LogOut, base::Unretained(this),
            on_failed, on_logged_out));
    return;
  }

  class LogoutCallback : public CefDeleteCookiesCallback {
   public:
    LogoutCallback(
        YouTube *caller,
        const OnFailed &on_failed,
        const OnLoggedOut &on_logged_out)
        : CefDeleteCookiesCallback{},
          caller_{caller},
          on_failed_{on_failed},
          on_logged_out_{on_logged_out} {}

    virtual ~LogoutCallback() {}

    void OnComplete(int /*num_deleted*/) override {
      caller_->SetAccessToken("");
      caller_->refresh_token_ = "";
      on_logged_out_();
    }

   private:
    YouTube *caller_;
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


void YouTube::PostLiveVideo(
    const std::string &stream_server,
    const std::string &user_page_id,
    const std::string &privacy,
    const std::string &title,
    const std::string &description,
    const std::string &app_attribution_tag,
    const OnFailed &on_failed,
    const OnLiveVideoPosted &on_live_video_posted) {
  GetBroadcast(
      on_failed, [this,
                  description,
                  privacy,
                  on_failed,
                  on_live_video_posted](const std::string &broadcast_id,
                                        const std::string &stream_id,
                                        const std::string &page_link) {
    UpdateBroadcast(
        broadcast_id,
        description,
        privacy,
        on_failed, [this,
                    on_failed,
                    stream_id,
                    page_link,
                    on_live_video_posted]() {
      GetStream(
          stream_id,
          on_failed,
          [page_link,
           on_live_video_posted](const std::string &stream_server,
                                 const std::string &stream_key) {
        on_live_video_posted(stream_server, stream_key, page_link);
      });
    });
  });
}


void YouTube::GetComments(const std::string &created_time,
    const OnFailed &on_failed,
    const OnCommentsGot &on_comments_got) {
  Uri chat_messages_list_uri {YouTubeApi::Graph::ChatMessagesList::BuildUri(
      GetLiveChatId(), GetAccessToken())};

  OutputDebugStringA(chat_messages_list_uri.uri_string().c_str());
  http_request_service_.Get(
      chat_messages_list_uri.uri_string(),
      [on_failed](const boost::system::error_code &ec) {
    std::string msg{ec.message()};
    on_failed(msg);
  }, [this, created_time, on_comments_got](const std::string &str) {
    OutputDebugStringA(str.c_str());

    // read json result
    std::deque<std::tuple<std::string, std::string, std::string,
        std::string>> reservoir;
    boost::property_tree::ptree tree;
    std::stringstream ss{str};
    try {
      boost::property_tree::read_json(ss, tree);
      const auto &items{tree.get_child("items")};

      for (const auto &item : items) {
        const std::string &id{
            item.second.get<std::string>("id")};
        const std::string &time{
            item.second.get<std::string>("snippet.publishedAt")};
        const std::string &msg{
          item.second.get<std::string>("snippet.displayMessage")};
        const std::string &nick{
          item.second.get<std::string>("authorDetails.displayName")};

        reservoir.emplace_front(id, time, nick, msg);
      }
    } catch (const std::exception &/*e*/) {
      reservoir.clear();
    }

    // make facebook result format json
    boost::property_tree::ptree root;
    boost::property_tree::ptree datas;
    for (const auto &chat : reservoir) {
      const std::string str_time = std::get<1>(chat);

      boost::posix_time::ptime chat_time = RefineISOTimeString(str_time);
      if (created_time.length() > 4) {
        boost::posix_time::ptime user_time =
            RefineISOTimeString(created_time);
        if (chat_time < user_time)
          break;
      }

      const std::string id = std::get<0>(chat);
      const std::string nick = std::get<2>(chat);
      const std::string msg = std::get<3>(chat);

      boost::property_tree::ptree from;
      from.add<std::string>("name", nick);

      boost::property_tree::ptree data;
      data.add<std::string>("created_time", str_time);
      data.add_child("from", from);
      data.add<std::string>("message", msg);
      data.add<std::string>("id", id);

      datas.push_back(std::make_pair("", data));
    }
    root.add_child("data", datas);
    root.add_child("paging", boost::property_tree::ptree());

    std::ostringstream oss;
    write_json(oss, root);
    on_comments_got(oss.str());
  });
}


void YouTube::GetLiveVideoViewers(
    const OnFailed &on_failed,
    const OnLiveVideoViewers &on_live_video_viewers) {
  Uri live_video_viewers_uri =
      YouTubeApi::Graph::CurrentViewerCount::BuildUrl(GetVideoId());

  OutputDebugStringA(live_video_viewers_uri.uri_string().c_str());
  http_request_service_.Get(
      live_video_viewers_uri.uri_string(),
      [on_failed](const boost::system::error_code &ec) {
    std::string msg{ec.message()};
    on_failed(msg);
  }, [on_live_video_viewers](const std::string &str) {
    OutputDebugStringA(str.c_str());

    boost::property_tree::ptree root;
    root.add<std::string>("live_views", str);
    std::ostringstream oss;
    write_json(oss, root);
    on_live_video_viewers(oss.str());
  });
}


void YouTube::StopLiveVideo() {
}


void YouTube::GetRefreshToken(const std::string &last_access_token) {
  if (last_access_token != GetAccessToken()) {
    return;
  }

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
                   base::Unretained(this),
                   GetAccessToken()),
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


void YouTube::GetBroadcast(
    const OnFailed &on_failed,
    const OnBroadcastGotten &on_braodcast_gotten) {
  Uri broadcast_list_uri{YouTubeApi::Graph::BroadcastList::BuildUri(
      GetAccessToken())};

  http_request_service_.Get(
      broadcast_list_uri.uri_string(),
      [on_failed](const boost::system::error_code &ec) {
    std::string msg{ec.message()};
    if (msg == "Forbidden") {
      on_failed("no channel or streaming service");
    } else {
      on_failed(msg);
    }
  }, [this, on_failed, on_braodcast_gotten](const std::string &str) {
    boost::property_tree::ptree tree;
    std::stringstream ss{str};
    using Broadcast = std::tuple<std::string, std::string, std::string,
        std::string>;
    std::vector<Broadcast> broadcasts;
    try {
      boost::property_tree::read_json(ss, tree);
      const auto &items{tree.get_child("items")};
      if (items.size() < 1) {
        on_failed("no broadcast items");
        return;
      }
      for (const auto &broadcast : items) {
        const std::string &broadcast_id =
            broadcast.second.get<std::string>("id");
        const std::string &stream_id =
            broadcast.second.get<std::string>(
                "contentDetails.boundStreamId");
        const std::string &embed_html =
            broadcast.second.get<std::string>(
                "contentDetails.monitorStream.embedHtml");
        const std::string link = ExtractLinkFromHtml(embed_html);
        const std::string &live_chat_id =
            broadcast.second.get<std::string>("snippet.liveChatId");

        broadcasts.emplace_back(
            std::make_tuple(broadcast_id, stream_id, link, live_chat_id));
      }
    } catch (const std::exception &/*e*/) {
      broadcasts.clear();
    }

    if (broadcasts.empty() == true) {
      std::stringstream msg;
      msg << "could not broadcast list from: " << str;
      on_failed(msg.str());
      return;
    }

    const std::string &broadcast_id = std::get<0>(broadcasts[0]);
    const std::string &stream_id = std::get<1>(broadcasts[0]);
    const std::string &page_link = std::get<2>(broadcasts[0]);
    const std::string &live_chat_id = std::get<3>(broadcasts[0]);

    SetLiveChatId(live_chat_id);
    SetVideoId(broadcast_id);

    on_braodcast_gotten(broadcast_id, stream_id, page_link);
  });
}


void YouTube::GetStream(
      const std::string &stream_id,
      const OnFailed &on_failed,
      const OnStreamGotten &on_stream_gotten) {
  Uri stream_name_uri{YouTubeApi::Graph::StreamList::BuildUri(
      stream_id,
      GetAccessToken())};

  http_request_service_.Get(
      stream_name_uri.uri_string(),
      [on_failed](const boost::system::error_code &ec) {
    std::string msg{ec.message()};
    on_failed(msg);
  }, [this, on_failed, on_stream_gotten](const std::string &str) {
    boost::property_tree::ptree tree;
    std::stringstream ss{str};
    using Stream = std::tuple<std::string, std::string>;
    std::vector<Stream> streams;
    try {
      boost::property_tree::read_json(ss, tree);
      const auto &items{tree.get_child("items")};
      if (items.size() < 1) {
        on_failed("no stream items");
        return;
      }

      for (const auto &stream : items) {
        const std::string &stream_server =
            stream.second.get<std::string>(
                "cdn.ingestionInfo.ingestionAddress");
        const std::string &stream_key =
            stream.second.get<std::string>(
                "cdn.ingestionInfo.streamName");
        streams.emplace_back(std::make_tuple(stream_server, stream_key));
      }
    } catch (const std::exception &/*e*/) {
      streams.clear();
    }

    if (streams.empty() == true) {
      std::stringstream msg;
      msg << "could not get stream server, key from: " << str;
      on_failed(msg.str());
      return;
    }
    on_stream_gotten(std::get<0>(streams[0]), std::get<1>(streams[0]));
  });
}


void YouTube::UpdateBroadcast(
    const std::string &broadcast_id,
    const std::string &title,
    const std::string &privacy_status,
    const OnFailed &on_failed,
    const OnBroadcastUpdated &on_broadcast_updated) {
  Uri video_update_uri{YouTubeApi::Graph::VideoUpdate::BuildUri(
      GetAccessToken())};

  boost::property_tree::ptree post_content{
      YouTubeApi::Graph::VideoUpdate::BuildPostContent(
          broadcast_id,
          title,
          privacy_status)};

  http_request_service_.Put(
      video_update_uri.uri_string(),
      post_content,
      [on_failed](const boost::system::error_code &ec) {
    std::string msg{ec.message()};
    on_failed(msg);
  }, [on_broadcast_updated](const std::string &str) {
    on_broadcast_updated();
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
                   base::Unretained(this),
                   GetAccessToken()),
        (expires_in - 10) * 1000);

    GetChannel(on_failed, [this, on_failed, on_logged_in](
        const std::string &user_name) {
      on_logged_in(user_name, {}, {});

      // check user agreement
      GetBroadcast(on_failed, [](
          const std::string &broadcast_id,
          const std::string &stream_id,
          const std::string &page_link) {
        // do nothing
      });
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


std::string YouTube::GetLiveChatId() const {
  std::string live_chat_id{};
  {
    std::lock_guard<std::mutex> lock{live_chat_id_mutex_};
    live_chat_id = live_chat_id_;
  }
  return live_chat_id;
}


void YouTube::SetLiveChatId(const std::string &live_chat_id) {
  std::lock_guard<std::mutex> lock{live_chat_id_mutex_};
  live_chat_id_ = live_chat_id;
}


std::string YouTube::GetVideoId() const {
  std::string video_id{};
  {
    std::lock_guard<std::mutex> lock{video_id_mutex_};
    video_id = video_id_;
  }
  return video_id;
}


void YouTube::SetVideoId(const std::string &video_id) {
  std::lock_guard<std::mutex> lock{video_id_mutex_};
  video_id_ = video_id;
}


boost::posix_time::ptime YouTube::RefineISOTimeString(
    const std::string &time) const {
  size_t endpos = time.find_first_of(".");
  std::string refined = time.substr(0, endpos);
  boost::erase_all(refined, "-");
  boost::erase_all(refined, ":");

  return boost::posix_time::from_iso_string(refined);
}


const std::string YouTube::ExtractLinkFromHtml(const std::string &html) {
  static const std::string kEmptyString;
  static const std::regex kUriPattern{R"(src=(\")(.*?)(\"))"};

  std::smatch matches;
  const bool &found = std::regex_search(html, matches, kUriPattern);

  return found == true ? matches[2] : kEmptyString;
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
