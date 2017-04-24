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


const urdl::http::request_content_type
    HttpRequestContentType::kApplicationJson{"application/json"};


void HttpRequestContent::SetJson(
    const boost::property_tree::ptree &content,
    urdl::read_stream *out) {
  std::stringstream json;
  boost::property_tree::write_json(json, content, false);

  out->set_option(HttpRequestContentType::kApplicationJson);
  out->set_option(urdl::http::request_content{json.str()});
}
}  // namespace ncstreamer
