/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_LIB_HTTP_TYPES_H_
#define NCSTREAMER_CEF_SRC_LIB_HTTP_TYPES_H_

#include <string>

#include "boost/property_tree/ptree.hpp"

#pragma warning(push)
#pragma warning(disable: 4244)
#include "urdl/istream.hpp"
#pragma warning(pop)


namespace ncstreamer {
class HttpRequestMethod {
 public:
  static const urdl::http::request_method kGet;
  static const urdl::http::request_method kHead;
  static const urdl::http::request_method kPost;
  static const urdl::http::request_method kPut;
  static const urdl::http::request_method kDelete;
};


class HttpRequestContent {
 public:
  static void SetEmpty(
      urdl::read_stream *out);

  static void SetJson(
      const std::string &content,
      urdl::read_stream *out);

  static void SetWwwFormUrlEncoded(
      const std::string &content,
      urdl::read_stream *out);
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_LIB_HTTP_TYPES_H_
