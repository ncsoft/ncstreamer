/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/streaming_service/facebook_api.h"

#include <codecvt>
#include <locale>
#include <sstream>
#include <utility>

#include "src/lib/string.h"


namespace ncstreamer {
const wchar_t *FacebookApi::kScheme{L"https"};
const wchar_t *FacebookApi::kVersion{L"v2.8"};


const wchar_t *FacebookApi::Login::kAuthority{L"www.facebook.com"};


Uri FacebookApi::Login::Oauth::BuildUri(
    const std::wstring &client_id,
    const Uri &redirect_uri,
    const std::wstring &response_type,
    const std::wstring &display,
    const std::vector<std::wstring> &scope) {
  return {kScheme, kAuthority, static_path(), Uri::Query{{
      {L"client_id", client_id},
      {L"redirect_uri", redirect_uri.uri_string()},
      {L"response_type", response_type},
      {L"display", display},
      {L"scope", String::Join(scope, L",")}}}};
}


const std::wstring &FacebookApi::Login::Oauth::static_path() {
  static const std::wstring kPath{[]() {
    std::wstringstream ss;
    ss << L"/" << kVersion << L"/dialog/oauth";
    return ss.str();
  }()};
  return kPath;
}


const Uri &FacebookApi::Login::Redirect::static_uri() {
  static const Uri kUri{kScheme, kAuthority, L"/connect/login_success.html"};
  return kUri;
}


std::wstring FacebookApi::Login::Redirect::ExtractAccessToken(
    const Uri::Query &query) {
  return query.GetParameter(L"access_token");
}


const wchar_t *FacebookApi::Graph::kAuthority{L"graph.facebook.com"};


const Uri &FacebookApi::Graph::Me::static_uri() {
  static const Uri kUri{kScheme, kAuthority, static_path()};
  return kUri;
}


Uri FacebookApi::Graph::Me::BuildUri(
    const std::wstring &access_token,
    const std::vector<std::wstring> &fields) {
  return {kScheme, kAuthority, static_path(), Uri::Query{{
      {L"access_token", access_token},
      {L"fields", String::Join(fields, L",")}}}};
}


const std::wstring &FacebookApi::Graph::Me::static_path() {
  static const std::wstring kPath{[]() {
    std::wstringstream ss;
    ss << L"/" << kVersion << L"/me";
    return ss.str();
  }()};
  return kPath;
}


Uri FacebookApi::Graph::LiveVideos::BuildUri(
    const std::wstring &user_page_id) {
  return {kScheme, kAuthority, BuildPath(user_page_id)};
}


boost::property_tree::ptree
    FacebookApi::Graph::LiveVideos::BuildPostContent(
        const std::wstring &access_token,
        const std::wstring &description) {
  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

  boost::property_tree::ptree post_content;
  post_content.add<std::string>(
      "access_token", converter.to_bytes(access_token));
  post_content.add<std::string>(
      "description", converter.to_bytes(description));
  return std::move(post_content);
}


std::wstring FacebookApi::Graph::LiveVideos::BuildPath(
    const std::wstring &user_page_id) {
  std::wstringstream ss;
  ss << L"/" << user_page_id << L"/live_videos";
  return ss.str();
}
}  // namespace ncstreamer
