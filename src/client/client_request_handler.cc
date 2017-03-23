/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/client/client_request_handler.h"

#include <cassert>
#include <codecvt>
#include <locale>
#include <regex>  // NOLINT
#include <unordered_map>
#include <utility>
#include <vector>

#include "Shlwapi.h"

#include "include/wrapper/cef_helpers.h"

#include "src/js_executor.h"
#include "src/obs.h"
#include "src/streaming_service.h"


namespace ncstreamer {
ClientRequestHandler::ClientRequestHandler() {
}


ClientRequestHandler::~ClientRequestHandler() {
}


bool ClientRequestHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefFrame> /*frame*/,
                                          CefRefPtr<CefRequest> request,
                                          bool /*is_redirect*/) {
  CEF_REQUIRE_UI_THREAD();

  std::wstring uri{request->GetURL()};

  // from https://tools.ietf.org/html/rfc3986#appendix-B
  static const std::wregex kUriPattern{
      LR"(^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)"};

  std::wsmatch matches;
  bool found = std::regex_search(uri, matches, kUriPattern);
  if (found) {
    std::wstring scheme{matches[2]};
    std::wstring host{matches[4]};
    std::wstring path{matches[5]};
    std::wstring query{matches[7]};
    if (scheme == L"cef") {
      OnCommand(host + path, ParseQuery(query), browser);
    } else {
      return false;  // proceed navigation.
    }
  }

  return true;  // cancel navigation.
}


ClientRequestHandler::CommandArgumentMap
    ClientRequestHandler::ParseQuery(const std::wstring &query) {
  CommandArgumentMap args;
  static const std::wregex kQueryPattern{LR"(([\w+%]+)=([^&]*))"};

  auto begin = std::wsregex_iterator(query.begin(), query.end(), kQueryPattern);
  auto end = std::wsregex_iterator();

  for (std::wsregex_iterator i = begin; i != end; ++i) {
    const auto &matches = *i;
    args.emplace(matches[1], DecodeUri(matches[2]));
  }

  return std::move(args);
}


std::wstring ClientRequestHandler::DecodeUri(const std::wstring &enc_string) {
  static const std::size_t kMaxSize{2048};

  if (enc_string.size() >= kMaxSize - 1) {
    return enc_string;
  }

  wchar_t buf[kMaxSize];
  std::wcsncpy(buf, enc_string.c_str(), enc_string.size() + 1);

  HRESULT result = ::UrlUnescapeInPlace(buf, 0);
  if (result != S_OK) {
    return enc_string;
  }

  return std::move(std::wstring{buf});
}


void ClientRequestHandler::OnCommand(const std::wstring &cmd,
                                     const CommandArgumentMap &args,
                                     CefRefPtr<CefBrowser> browser) {
  using This = ClientRequestHandler;
  static const std::unordered_map<std::wstring/*command*/,
                                  CommandHandler> kCommandHandlers{
      {L"service_provider/log_in",
       std::bind(&This::OnCommandServiceProviderLogIn, this,
           std::placeholders::_1, std::placeholders::_2)},
      {L"streaming/start",
       std::bind(&This::OnCommandStreamingStart, this,
           std::placeholders::_1, std::placeholders::_2)},
      {L"streaming/stop",
       std::bind(&This::OnCommandStreamingStop, this,
           std::placeholders::_1, std::placeholders::_2)},
      {L"settings/mic/on",
           std::bind(&This::OnCommandSettingsMicOn, this,
           std::placeholders::_1, std::placeholders::_2)},
      {L"settings/mic/off",
           std::bind(&This::OnCommandSettingsMicOff, this,
           std::placeholders::_1, std::placeholders::_2)},
      {L"settings/video_quality/update",
       std::bind(&This::OnCommandSettingsVideoQualityUpdate, this,
           std::placeholders::_1, std::placeholders::_2)}};

  auto i = kCommandHandlers.find(cmd);
  if (i == kCommandHandlers.end()) {
    assert(false);
    return;
  }

  i->second(args, browser);
}


void ClientRequestHandler::OnCommandServiceProviderLogIn(
    const CommandArgumentMap &args, CefRefPtr<CefBrowser> browser) {
  auto provider_i = args.find(L"serviceProvider");
  if (provider_i == args.end()) {
    assert(false);
    return;
  }

  const std::wstring &service_provider = provider_i->second;

  StreamingService::Get()->LogIn(
      service_provider,
      browser->GetHost()->GetWindowHandle(),
      [](const std::wstring &error) {
  }, [browser](
      const std::wstring &user_name,
      const std::vector<std::wstring> &user_pages) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::vector<std::string> utf8_pages;
    for (const auto &page : user_pages) {
      utf8_pages.emplace_back(converter.to_bytes(page));
    }
    JsExecutor::Execute(
        browser,
        "cef.onResponse",
        std::make_pair("userName", converter.to_bytes(user_name)),
        std::make_pair("userPages", utf8_pages));
  });
}


void ClientRequestHandler::OnCommandStreamingStart(
    const CommandArgumentMap &args, CefRefPtr<CefBrowser> browser) {
  auto source_i = args.find(L"source");
  auto provider_i = args.find(L"serviceProvider");
  auto url_i = args.find(L"streamUrl");
  if (source_i == args.end() ||
      provider_i == args.end() ||
      url_i == args.end()) {
    assert(false);
    return;
  }

  const std::wstring &source = source_i->second;
  const std::wstring &service_provider = provider_i->second;
  const std::wstring &stream_url = url_i->second;

  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  Obs::Get()->StartStreaming(
      converter.to_bytes(source),
      converter.to_bytes(service_provider),
      converter.to_bytes(stream_url),
      [browser]() {
        JsExecutor::Execute(browser, "onStreamingStarted");
      });
}


void ClientRequestHandler::OnCommandStreamingStop(
    const CommandArgumentMap &/*args*/, CefRefPtr<CefBrowser> browser) {
  Obs::Get()->StopStreaming([browser]() {
    JsExecutor::Execute(browser, "onStreamingStopped");
  });
}


void ClientRequestHandler::OnCommandSettingsMicOn(
  const CommandArgumentMap &/*args*/, CefRefPtr<CefBrowser> /*browser*/) {
  Obs::Get()->TurnOnMic();
}


void ClientRequestHandler::OnCommandSettingsMicOff(
  const CommandArgumentMap &/*args*/, CefRefPtr<CefBrowser> /*browser*/) {
  Obs::Get()->TurnOffMic();
}


void ClientRequestHandler::OnCommandSettingsVideoQualityUpdate(
    const CommandArgumentMap &args, CefRefPtr<CefBrowser> browser) {
  auto width_i = args.find(L"width");
  auto height_i = args.find(L"height");
  auto fps_i = args.find(L"fps");
  auto bitrate_i = args.find(L"bitrate");
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
