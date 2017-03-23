/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/lib/uri.h"

#include <sstream>

#include "include/cef_parser.h"


namespace ncstreamer {
std::wstring Uri::ToString(
    const std::wstring &scheme,
    const std::pair<std::wstring, std::wstring> &authority,
    const std::wstring &path,
    const Query &query) {
  CefURLParts parts;
  CefString(&parts.scheme) = scheme.c_str();
  CefString(&parts.host) = authority.first.c_str();
  CefString(&parts.port) = authority.second.c_str();
  CefString(&parts.path) = path.c_str();
  CefString(&parts.query) = ToString(query).c_str();

  CefString uri;
  bool result = ::CefCreateURL(parts, uri);
  if (result == false) {
    return L"";
  }

  return uri;
}


std::wstring Uri::ToString(const Query &query) {
  if (query.empty() == true) {
    return L"";
  }

  static const auto to_string = [](const Query::const_iterator &i) {
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
