/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/client/client_request_handler.h"

#include <codecvt>
#include <locale>
#include <regex>  // NOLINT
#include <unordered_map>
#include <utility>

#include "Shlwapi.h"

#include "include/wrapper/cef_helpers.h"

#include "src/js_executor.h"
#include "src/obs.h"


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
    if (scheme == L"command") {
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
      {L"streaming/start",
       std::bind(&This::OnCommandStreamingStart, this,
           std::placeholders::_1, std::placeholders::_2)},
      {L"streaming/stop",
       std::bind(&This::OnCommandStreamingStop, this,
           std::placeholders::_1, std::placeholders::_2)}};

  auto i = kCommandHandlers.find(cmd);
  if (i == kCommandHandlers.end()) {
    return;
  }

  i->second(args, browser);
}


void ClientRequestHandler::OnCommandStreamingStart(
    const CommandArgumentMap &args, CefRefPtr<CefBrowser> browser) {
  auto source_i = args.find(L"source");
  auto provider_i = args.find(L"serviceProvider");
  auto url_i = args.find(L"streamUrl");
  if (source_i == args.end() ||
      provider_i == args.end() ||
      url_i == args.end()) {
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
}  // namespace ncstreamer
