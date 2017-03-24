/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/streaming_service/facebook_api.h"

#include <sstream>


namespace ncstreamer {
const wchar_t *FacebookApi::kScheme{L"https"};
const wchar_t *FacebookApi::kVersion{L"v2.8"};


const wchar_t *FacebookApi::Login::kAuthority{L"www.facebook.com"};


Uri FacebookApi::Login::Oauth::BuildUri(
    const std::wstring &client_id,
    const Uri &redirect_uri,
    const std::wstring &response_type,
    const std::wstring &display) {
  return {kScheme, kAuthority, static_path(), Uri::Query{{
      {L"client_id", client_id},
      {L"redirect_uri", redirect_uri.uri_string()},
      {L"response_type", response_type},
      {L"display", display}}}};
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
}  // namespace ncstreamer
