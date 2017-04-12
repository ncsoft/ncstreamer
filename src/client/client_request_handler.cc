/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/client/client_request_handler.h"

#include <cassert>
#include <regex>  // NOLINT
#include <unordered_map>
#include <utility>
#include <vector>

#include "boost/property_tree/ptree.hpp"
#include "include/wrapper/cef_helpers.h"

#include "Shellapi.h"  // NOLINT
#include "Shlwapi.h"  // NOLINT

#include "src/js_executor.h"
#include "src/obs.h"
#include "src/streaming_service.h"


namespace ncstreamer {
ClientRequestHandler::ClientRequestHandler(
    const std::wstring &locale)
    : locale_{locale} {
}


ClientRequestHandler::~ClientRequestHandler() {
}


bool ClientRequestHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefFrame> /*frame*/,
                                          CefRefPtr<CefRequest> request,
                                          bool /*is_redirect*/) {
  CEF_REQUIRE_UI_THREAD();

  std::string uri{request->GetURL()};

  // from https://tools.ietf.org/html/rfc3986#appendix-B
  static const std::regex kUriPattern{
      R"(^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)"};

  std::smatch matches;
  bool found = std::regex_search(uri, matches, kUriPattern);
  if (found) {
    std::string scheme{matches[2]};
    std::string host{matches[4]};
    std::string path{matches[5]};
    std::string query{matches[7]};
    if (scheme == "cef") {
      OnCommand(host + path, ParseQuery(query), browser);
    } else {
      return false;  // proceed navigation.
    }
  }

  return true;  // cancel navigation.
}


ClientRequestHandler::CommandArgumentMap
    ClientRequestHandler::ParseQuery(const std::string &query) {
  CommandArgumentMap args;
  static const std::regex kQueryPattern{R"(([\w+%]+)=([^&]*))"};

  auto begin = std::sregex_iterator(query.begin(), query.end(), kQueryPattern);
  auto end = std::sregex_iterator();

  for (std::sregex_iterator i = begin; i != end; ++i) {
    const auto &matches = *i;
    args.emplace(matches[1], DecodeUri(matches[2]));
  }

  return std::move(args);
}


std::string ClientRequestHandler::DecodeUri(const std::string &enc_string) {
  static const std::size_t kMaxSize{2048};

  if (enc_string.size() >= kMaxSize - 1) {
    return enc_string;
  }

  char buf[kMaxSize];
  std::strncpy(buf, enc_string.c_str(), enc_string.size() + 1);

  HRESULT result = ::UrlUnescapeA(buf, NULL, NULL, URL_UNESCAPE_INPLACE);
  if (result != S_OK) {
    return enc_string;
  }

  return std::move(std::string{buf});
}


void ClientRequestHandler::OnCommand(const std::string &cmd,
                                     const CommandArgumentMap &args,
                                     CefRefPtr<CefBrowser> browser) {
  using This = ClientRequestHandler;
  static const std::unordered_map<std::string/*command*/,
                                  CommandHandler> kCommandHandlers{
      {"window/close",
       std::bind(&This::OnCommandWindowClose, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"window/minimize",
       std::bind(&This::OnCommandWindowMinimize, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"external_browser/pop_up",
       std::bind(&This::OnCommandExternalBrowserPopUp, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"service_provider/log_in",
       std::bind(&This::OnCommandServiceProviderLogIn, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"streaming/start",
       std::bind(&This::OnCommandStreamingStart, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"streaming/stop",
       std::bind(&This::OnCommandStreamingStop, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"settings/mic/on",
           std::bind(&This::OnCommandSettingsMicOn, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"settings/mic/off",
           std::bind(&This::OnCommandSettingsMicOff, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"settings/video_quality/update",
       std::bind(&This::OnCommandSettingsVideoQualityUpdate, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)}};

  auto i = kCommandHandlers.find(cmd);
  if (i == kCommandHandlers.end()) {
    assert(false);
    return;
  }

  i->second(cmd, args, browser);
}


void ClientRequestHandler::OnCommandWindowClose(
    const std::string &/*cmd*/,
    const CommandArgumentMap &/*args*/,
    CefRefPtr<CefBrowser> browser) {
  browser->GetHost()->CloseBrowser(true);
}


void ClientRequestHandler::OnCommandWindowMinimize(
    const std::string &/*cmd*/,
    const CommandArgumentMap &/*args*/,
    CefRefPtr<CefBrowser> browser) {
  HWND wnd{browser->GetHost()->GetWindowHandle()};
  ::ShowWindow(wnd, SW_MINIMIZE);
}


void ClientRequestHandler::OnCommandExternalBrowserPopUp(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  auto uri_i = args.find("uri");
  if (uri_i == args.end()) {
    assert(false);
    return;
  }

  const std::string &uri = uri_i->second;
  ::ShellExecuteA(NULL, "open", uri.c_str(), NULL, NULL, SW_SHOWNORMAL);
}


void ClientRequestHandler::OnCommandServiceProviderLogIn(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  auto provider_i = args.find("serviceProvider");
  if (provider_i == args.end()) {
    assert(false);
    return;
  }

  const std::string &service_provider = provider_i->second;

  StreamingService::Get()->LogIn(
      service_provider,
      browser->GetHost()->GetWindowHandle(),
      locale_,
      [](const std::string &error) {
    // TODO(khpark): TBD
  }, [browser, cmd](
      const std::string &user_name,
      const std::string &user_link,
      const std::vector<StreamingServiceProvider::UserPage> &user_pages) {
    std::vector<boost::property_tree::ptree> tree_pages;
    for (const auto &page : user_pages) {
      tree_pages.emplace_back(page.ToTree());
    }
    JsExecutor::Execute<boost::property_tree::ptree>(
        browser,
        "cef.onResponse",
        cmd,
        std::make_pair("userName", user_name),
        std::make_pair("userLink", user_link),
        std::make_pair("userPages", tree_pages));
  });
}


void ClientRequestHandler::OnCommandStreamingStart(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  auto source_i = args.find("source");
  auto user_page_i = args.find("userPage");
  auto privacy_i = args.find("privacy");
  auto title_i = args.find("title");
  auto description_i = args.find("description");
  if (source_i == args.end() ||
      user_page_i == args.end() ||
      privacy_i == args.end() ||
      title_i == args.end() ||
      description_i == args.end()) {
    assert(false);
    return;
  }

  const std::string &source = source_i->second;
  const std::string &user_page = user_page_i->second;
  const std::string &privacy = privacy_i->second;
  const std::string &title = title_i->second;
  const std::string &description = description_i->second;

  if (source.empty() == true ||
      user_page.empty() == true ||
      privacy.empty() == true) {
    assert(false);
    return;
  }

  StreamingService::Get()->PostLiveVideo(
      user_page,
      privacy,
      title,
      description,
      [](const std::string &error) {
    // TODO(khpark): TBD
  }, [browser, cmd, source](const std::string &service_provider,
                            const std::string &stream_url) {
    Obs::Get()->StartStreaming(
        source,
        service_provider,
        stream_url,
        [browser, cmd]() {
      JsExecutor::Execute(
          browser,
          "cef.onResponse",
          cmd);
    });
  });
}


void ClientRequestHandler::OnCommandStreamingStop(
    const std::string &cmd,
    const CommandArgumentMap &/*args*/,
    CefRefPtr<CefBrowser> browser) {
  Obs::Get()->StopStreaming([browser, cmd]() {
    JsExecutor::Execute(
        browser,
        "cef.onResponse",
        cmd);
  });
}


void ClientRequestHandler::OnCommandSettingsMicOn(
    const std::string &cmd,
    const CommandArgumentMap &/*args*/,
    CefRefPtr<CefBrowser> /*browser*/) {
  Obs::Get()->TurnOnMic();
}


void ClientRequestHandler::OnCommandSettingsMicOff(
    const std::string &cmd,
    const CommandArgumentMap &/*args*/,
    CefRefPtr<CefBrowser> /*browser*/) {
  Obs::Get()->TurnOffMic();
}


void ClientRequestHandler::OnCommandSettingsVideoQualityUpdate(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  auto width_i = args.find("width");
  auto height_i = args.find("height");
  auto fps_i = args.find("fps");
  auto bitrate_i = args.find("bitrate");
  if (width_i == args.end() ||
      height_i == args.end() ||
      fps_i == args.end() ||
      bitrate_i == args.end()) {
    assert(false);
    return;
  }

  uint32_t width{0};
  uint32_t height{0};
  uint32_t fps{0};
  uint32_t bitrate{0};
  try {
    width = std::stoul(width_i->second);
    height = std::stoul(height_i->second);
    fps = std::stoul(fps_i->second);
    bitrate = std::stoul(bitrate_i->second);
  } catch (...) {
  }

  if (width == 0 ||
      height == 0 ||
      fps == 0 ||
      bitrate == 0) {
    assert(false);
    return;
  }

  Obs::Get()->UpdateVideoQuality({width, height}, fps, bitrate);
}
}  // namespace ncstreamer
