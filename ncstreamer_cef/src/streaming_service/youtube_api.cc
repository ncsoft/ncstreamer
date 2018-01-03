/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#include "ncstreamer_cef/src/streaming_service/youtube_api.h"

#include <sstream>

#include "include/cef_parser.h"

#include "ncstreamer_cef/src/lib/string.h"


namespace ncstreamer {
const char *YouTubeApi::kScheme{"https"};


const char *YouTubeApi::Login::Oauth::kAuthority{
    "accounts.google.com/o/oauth2"};


Uri YouTubeApi::Login::Oauth::Code::BuildUri(
    const std::string &client_id,
    const Uri &redirect_uri,
    const std::vector<std::string> &scope) {
  return {kScheme, kAuthority, static_path(), Uri::Query{{
      {"client_id", client_id},
      {"redirect_uri", redirect_uri.uri_string()},
      {"response_type", "code"},
      {"access_type", "offline"},
      {"scope", String::Join(scope, " ")}}}};
}


const std::string &YouTubeApi::Login::Oauth::Code::static_path() {
  static const std::string kPath{[]() {
    std::stringstream ss;
    ss << "/auth";
    return ss.str();
  }()};
  return kPath;
}


Uri YouTubeApi::Login::Oauth::RefreshToken::BuildUri() {
  return {kScheme, kAuthority, static_path()};
}


const std::string &YouTubeApi::Login::Oauth::RefreshToken::static_path() {
  static const std::string kPath{[]() {
    std::stringstream ss;
    ss << "/token";
    return ss.str();
  }()};
  return kPath;
}


const char *YouTubeApi::Login::Oauth::Token::kAuthority{
    "www.googleapis.com/oauth2/v4"};


Uri YouTubeApi::Login::Oauth::Token::BuildUri() {
  return {kScheme, kAuthority, static_path()};
}


const std::string &YouTubeApi::Login::Oauth::Token::static_path() {
  static const std::string kPath{[]() {
    std::stringstream ss;
    ss << "/token";
    return ss.str();
  }()};
  return kPath;
}


const Uri &YouTubeApi::Login::Redirect::static_uri() {
  static const Uri kUri{"http://localhost/"};
  return kUri;
}


std::string YouTubeApi::Login::Redirect::ExtractAccessCode(
    const Uri::Query &query) {
  const std::string &code = query.GetParameter("code");
  return ::CefURIDecode(code, false, UU_PATH_SEPARATORS);
}


const char *YouTubeApi::Graph::kAuthority{"www.googleapis.com/youtube/v3"};


Uri YouTubeApi::Graph::Channel::BuildUri(
    const std::string &access_token) {
  return {kScheme, kAuthority, static_path(), Uri::Query{{
      {"part", "snippet"},
      {"mine", "true"},
      {"access_token", access_token}}}};
}


const std::string &YouTubeApi::Graph::Channel::static_path() {
  static const std::string kPath{[]() {
    std::stringstream ss;
    ss << "/channels";
    return ss.str();
  }()};
  return kPath;
}
}  // namespace ncstreamer
