/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/client/client_request_handler.h"

#include <regex>  // NOLINT
#include <sstream>
#include <unordered_map>
#include <utility>

#include "Shlwapi.h"

#include "include/wrapper/cef_helpers.h"


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
    if (scheme == "command") {
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
  std::strncpy(buf, enc_string.c_str(), enc_string.size());

  HRESULT result = ::UrlUnescapeInPlace(buf, 0);
  if (result != S_OK) {
    return enc_string;
  }

  return std::move(std::string{buf});
}


void ClientRequestHandler::ExecuteJs(
    CefRefPtr<CefBrowser> browser,
    const std::string &controller,
    const std::string &func_name,
    const std::vector<std::string> &args) {
  std::stringstream js;
  js << "angular.element('[ng-controller=" << controller
     << "]').scope()." << func_name
     << "(";

  if (!args.empty()) {
    auto i = args.begin();
    js << "'" << *i << "'";
    for (++i; i != args.end(); ++i) {
      js << ", ";
      js << "'" << *i << "'";
    }
  }

  js << ")";

  browser->GetMainFrame()->ExecuteJavaScript(js.str(), "", 0);
}


void ClientRequestHandler::OnCommand(const std::string &cmd,
                                     const CommandArgumentMap &args,
                                     CefRefPtr<CefBrowser> browser) {
  using This = ClientRequestHandler;
  static const std::unordered_map<std::string/*command*/,
                                  CommandHandler> kCommandHandlers{};
  auto i = kCommandHandlers.find(cmd);
  if (i == kCommandHandlers.end()) {
    return;
  }

  i->second(args, browser);
}
}  // namespace ncstreamer
