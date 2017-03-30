/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_LIB_HTTP_TYPES_H_
#define SRC_LIB_HTTP_TYPES_H_


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


class HttpRequestContentType {
 public:
  static const urdl::http::request_content_type kApplicationJson;
};


class HttpRequestContent {
 public:
  static void SetJson(
      const boost::property_tree::ptree &content,
      urdl::read_stream *out);
};
}  // namespace ncstreamer


#endif  // SRC_LIB_HTTP_TYPES_H_
