/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_LIB_HTTP_TYPES_H_
#define SRC_LIB_HTTP_TYPES_H_


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
}  // namespace ncstreamer


#endif  // SRC_LIB_HTTP_TYPES_H_
