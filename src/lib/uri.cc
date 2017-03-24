/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/lib/uri.h"

#include <regex>  // NOLINT
#include <sstream>

#include "include/cef_parser.h"


namespace ncstreamer {
Uri::Query::Query(const std::wstring &query_string)
    : query_string_{query_string},
      params_{} {
  static const std::wregex kParamPattern{LR"(([\w+%]+)=([^&]*))"};

  for (std::wsregex_iterator i{
      query_string.begin(), query_string.end(), kParamPattern};
      i != std::wsregex_iterator{}; ++i) {
    const auto &elem = *i;
    const std::wstring &key = elem[1].str();
    const std::wstring &value = elem[2].str();
    params_.emplace(key, value);
  }
}


Uri::Query::Query()
    : query_string_{},
      params_{} {
}


Uri::Query::~Query() {
}


const std::wstring &Uri::Query::GetParameter(const std::wstring &key) const {
  static const std::wstring kEmpty{};

  auto i = params_.find(key);
  return (i != params_.end()) ? i->second : kEmpty;
}


Uri::Uri(const std::wstring &uri_string)
    : uri_string_{uri_string},
      scheme_{},
      authority_{},
      path_{},
      query_{},
      fragment_{} {
  // from https://tools.ietf.org/html/rfc3986#appendix-B
  static const std::wregex kUriPattern{
      LR"(^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)"};

  std::wsmatch matches;
  bool found = std::regex_search(uri_string, matches, kUriPattern);
  if (found) {
    scheme_ = matches[2];
    authority_ = matches[4];
    path_ = matches[5];
    query_ = Query{matches[7]};
    fragment_ = matches[9];
  }
}


Uri::~Uri() {
}


std::wstring Uri::ToString(
    const std::wstring &scheme_authority_path,
    const QueryParamVector &query,
    const std::wstring &fragment) {
  std::wstringstream ss;
  ss << scheme_authority_path;
  if (query.empty() == false) {
    ss << L"?" << ToString(query);
  }
  if (fragment.empty() == false) {
    ss << L"#" << fragment;
  }
  return ss.str();
}


std::wstring Uri::ToString(
    const std::wstring &scheme_authority_path,
    const QueryParamVector &query) {
  return ToString(scheme_authority_path, query, L"");
}


std::wstring Uri::ToString(
    const std::wstring &scheme,
    const std::wstring &authority,
    const std::wstring &path,
    const QueryParamVector &query,
    const std::wstring &fragment) {
  std::wstringstream ss;
  ss << scheme << L"://";
  ss << authority;
  if (path.empty() == false) {
    ss << path;
  }
  return ToString(ss.str(), query, fragment);
}


std::wstring Uri::ToString(
    const std::wstring &scheme,
    const std::wstring &authority,
    const std::wstring &path,
    const QueryParamVector &query) {
  return ToString(scheme, authority, path, query, L"");
}


std::wstring Uri::ToString(
    const std::wstring &scheme,
    const std::wstring &authority,
    const std::wstring &path) {
  return ToString(scheme, authority, path, {}, L"");
}


std::wstring Uri::ToString(const QueryParamVector &query) {
  if (query.empty() == true) {
    return L"";
  }

  static const auto to_string = [](const QueryParamVector::const_iterator &i) {
    std::wstringstream ss;
    ss << i->first << L"=" << Encode(i->second);
    return ss.str();
  };

  std::wstringstream ss;
  auto i = query.begin();
  ss << to_string(i);
  for (++i; i != query.end(); ++i) {
    ss << L"&" << to_string(i);
  }
  return ss.str();
}


std::wstring Uri::Encode(const std::wstring &raw) {
  return ::CefURIEncode(raw, false).c_str();
}
}  // namespace ncstreamer
