/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/lib/uri.h"

#include <sstream>

#include "include/cef_parser.h"


namespace ncstreamer {
std::wstring Uri::ToString(
    const std::wstring &scheme,
    const std::wstring &authority,
    const std::wstring &path,
    const Query &query) {
  std::wstringstream ss;
  ss << scheme << L"://";
  ss << authority;
  if (path.empty() == false) {
    ss << path;
  }
  if (query.empty() == false) {
    ss << L"?" << ToString(query);
  }
  return ss.str();
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
