/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/lib/uri.h"

#include <regex>  // NOLINT
#include <sstream>

#include "include/cef_parser.h"


namespace ncstreamer {
Uri::Query::Query(const ParamVector &params)
    : query_string_{ToString(params)},
      params_{} {
  for (const auto &param : params) {
    params_.emplace(param);
  }
}


Uri::Query::Query(const std::string &query_string)
    : query_string_{query_string},
      params_{} {
  static const std::regex kParamPattern{R"(([\w+%]+)=([^&]*))"};

  for (std::sregex_iterator i{
      query_string.begin(), query_string.end(), kParamPattern};
      i != std::sregex_iterator{}; ++i) {
    const auto &elem = *i;
    const std::string &key = elem[1].str();
    const std::string &value = elem[2].str();
    params_.emplace(key, value);
  }
}


Uri::Query::Query()
    : query_string_{},
      params_{} {
}


Uri::Query::~Query() {
}


std::string Uri::Query::ToString(const ParamVector &query) {
  if (query.empty() == true) {
    return "";
  }

  static const auto to_string = [](const ParamVector::const_iterator &i) {
    std::stringstream ss;
    ss << i->first << "=" << Encode(i->second);
    return ss.str();
  };

  std::stringstream ss;
  auto i = query.begin();
  ss << to_string(i);
  for (++i; i != query.end(); ++i) {
    ss << "&" << to_string(i);
  }
  return ss.str();
}


const std::string &Uri::Query::GetParameter(const std::string &key) const {
  static const std::string kEmpty{};

  auto i = params_.find(key);
  return (i != params_.end()) ? i->second : kEmpty;
}


std::string Uri::Query::Encode(const std::string &raw) {
  return std::string{::CefURIEncode(raw, false)};
}


std::size_t Uri::Hasher::operator()(const Uri &uri) const {
  return std::hash<std::string>()(uri.uri_string());
}


Uri::Uri(
    const std::string &scheme,
    const std::string &authority,
    const std::string &path,
    const Query &query,
    const std::string &fragment)
    : scheme_{scheme},
      authority_{authority},
      path_{path},
      query_{query},
      fragment_{fragment},
      scheme_authority_path_{ToString(scheme, authority, path, {}, "")},
      uri_string_{ToString(scheme_authority_path_, query, fragment)} {
}


Uri::Uri(
    const std::string &scheme,
    const std::string &authority,
    const std::string &path,
    const Query &query)
    : scheme_{scheme},
      authority_{authority},
      path_{path},
      query_{query},
      fragment_{},
      scheme_authority_path_{ToString(scheme, authority, path, {}, "")},
      uri_string_{ToString(scheme_authority_path_, query, "")} {
}


Uri::Uri(
    const std::string &scheme,
    const std::string &authority,
    const std::string &path)
    : scheme_{scheme},
      authority_{authority},
      path_{path},
      query_{},
      fragment_{},
      scheme_authority_path_{ToString(scheme, authority, path, {}, "")},
      uri_string_{scheme_authority_path_} {
}


Uri::Uri(const std::string &uri_string)
    : scheme_{},
      authority_{},
      path_{},
      query_{},
      fragment_{},
      scheme_authority_path_{},
      uri_string_{uri_string} {
  // from https://tools.ietf.org/html/rfc3986#appendix-B
  static const std::regex kUriPattern{
      R"(^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)"};

  std::smatch matches;
  bool found = std::regex_search(uri_string, matches, kUriPattern);
  if (found) {
    scheme_ = matches[2];
    authority_ = matches[4];
    path_ = matches[5];
    query_ = Query{matches[7]};
    fragment_ = matches[9];
  }
  scheme_authority_path_ = ToString(scheme_, authority_, path_, {}, "");
}


Uri::~Uri() {
}


std::string Uri::ToString(
    const std::string &scheme_authority_path,
    const Query &query,
    const std::string &fragment) {
  std::stringstream ss;
  ss << scheme_authority_path;
  if (query.query_string().empty() == false) {
    ss << "?" << query.query_string();
  }
  if (fragment.empty() == false) {
    ss << "#" << fragment;
  }
  return ss.str();
}


std::string Uri::ToString(
    const std::string &scheme,
    const std::string &authority,
    const std::string &path,
    const Query &query,
    const std::string &fragment) {
  std::stringstream ss;
  ss << scheme << "://";
  ss << authority;
  if (path.empty() == false) {
    ss << path;
  }
  return ToString(ss.str(), query, fragment);
}
}  // namespace ncstreamer
