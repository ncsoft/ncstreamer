/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/lib/http_types.h"

#include <sstream>

#include "boost/property_tree/json_parser.hpp"


namespace ncstreamer {
const urdl::http::request_method HttpRequestMethod::kGet{"GET"};
const urdl::http::request_method HttpRequestMethod::kHead{"HEAD"};
const urdl::http::request_method HttpRequestMethod::kPost{"POST"};
const urdl::http::request_method HttpRequestMethod::kPut{"PUT"};
const urdl::http::request_method HttpRequestMethod::kDelete{"DELETE"};


void HttpRequestContent::SetEmpty(
    urdl::read_stream *out) {
  static const urdl::http::request_content_type kEmptyType;

  out->set_option(kEmptyType);
  out->set_option(urdl::http::request_content{""});
}


void HttpRequestContent::SetJson(
    const std::string &content,
    urdl::read_stream *out) {
  static const urdl::http::request_content_type kApplicationJson{
      "application/json"};

  out->set_option(kApplicationJson);
  out->set_option(urdl::http::request_content{content.c_str()});
}


void HttpRequestContent::SetWwwFormUrlEncoded(
    const std::string &content,
    urdl::read_stream *out) {
  static const urdl::http::request_content_type kWwwFromUrlEncoded{
      "application/x-www-form-urlencoded"};

  out->set_option(kWwwFromUrlEncoded);
  out->set_option(urdl::http::request_content{content.c_str()});
}
}  // namespace ncstreamer
