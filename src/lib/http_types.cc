/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/lib/http_types.h"


namespace ncstreamer {
const urdl::http::request_method HttpRequestMethod::kGet{"GET"};
const urdl::http::request_method HttpRequestMethod::kHead{"HEAD"};
const urdl::http::request_method HttpRequestMethod::kPost{"POST"};
const urdl::http::request_method HttpRequestMethod::kPut{"PUT"};
const urdl::http::request_method HttpRequestMethod::kDelete{"DELETE"};
}  // namespace ncstreamer
