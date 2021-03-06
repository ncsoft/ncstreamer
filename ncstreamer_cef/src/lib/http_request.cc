/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/lib/http_request.h"

#include "ncstreamer_cef/src/lib/http_types.h"


namespace ncstreamer {
HttpRequest::HttpRequest(boost::asio::io_service *svc)
    : rstream_{*svc},
      out_{},
      buffer_{},
      err_handler_{},
      open_handler_{},
      read_handler_{},
      complete_handler_{},
      ostream_close_handler_{} {
  rstream_.set_option(urdl::ssl::ca_cert{"cacert.pem"});
}


void HttpRequest::Download(
    const urdl::url &url,
    const std::string &file_name,
    const ErrorHandler &err_handler,
    const OpenHandler &open_handler,
    const ReadHandler &read_handler,
    const DownloadCompleteHandler &complete_handler) {
  if (rstream_.is_open()) {
    // borrowed error code from boost::asio::error temporarily.
    // TODO(khpark): replace this error code with in-house one.
    err_handler(boost::asio::error::already_started);
    return;
  }

  rstream_.set_option(HttpRequestMethod::kGet);

  std::fstream *ofstream{new std::fstream{
      file_name.c_str(), std::ios_base::out | std::ios_base::binary}};
  out_.reset(ofstream);

  err_handler_ = err_handler;
  open_handler_ = open_handler;
  read_handler_ = read_handler;
  complete_handler_ = [complete_handler]() {
    complete_handler();
  };

  ostream_close_handler_ = [ofstream]() {
    ofstream->close();
  };

  Request(url);
}


void HttpRequest::Request(
    const urdl::url &url,
    const urdl::http::request_method &method,
    const HttpHeaderContentType &content_type,
    const std::string &post_content,
    const ErrorHandler &err_handler,
    const OpenHandler &open_handler,
    const ReadHandler &read_handler,
    const ResponseCompleteHandler &complete_handler) {
  if (rstream_.is_open()) {
    // borrowed error code from boost::asio::error temporarily.
    // TODO(khpark): replace this error code with in-house one.
    err_handler(boost::asio::error::already_started);
    return;
  }

  rstream_.set_option(method);
  switch (content_type) {
    case HttpHeaderContentType::kApplicationJson:
      HttpRequestContent::SetJson(post_content, &rstream_);
      break;
    case HttpHeaderContentType::kWwwFormUrlEncoded:
      HttpRequestContent::SetWwwFormUrlEncoded(post_content, &rstream_);
      break;
    default:
      HttpRequestContent::SetEmpty(&rstream_);
      break;
  }

  std::stringstream *stringstream{new std::stringstream{}};
  out_.reset(stringstream);

  err_handler_ = err_handler;
  open_handler_ = open_handler;
  read_handler_ = read_handler;
  complete_handler_ = [complete_handler, stringstream]() {
    complete_handler(stringstream->str());
  };

  static const OstreamCloseHandler kDefaultOstreamCloseHandler{[]() {}};
  ostream_close_handler_ = kDefaultOstreamCloseHandler;

  Request(url);
}


void HttpRequest::Get(
    const urdl::url &url,
    const ErrorHandler &err_handler,
    const OpenHandler &open_handler,
    const ReadHandler &read_handler,
    const ResponseCompleteHandler &complete_handler) {
  static const std::string kEmptyPostContent;

  Request(
      url,
      HttpRequestMethod::kGet,
      HttpHeaderContentType::kNone,
      kEmptyPostContent,
      err_handler,
      open_handler,
      read_handler,
      complete_handler);
}


void HttpRequest::Post(
    const urdl::url &url,
    const HttpHeaderContentType &content_type,
    const std::string &post_content,
    const ErrorHandler &err_handler,
    const OpenHandler &open_handler,
    const ReadHandler &read_handler,
    const ResponseCompleteHandler &complete_handler) {
  Request(
      url,
      HttpRequestMethod::kPost,
      content_type,
      post_content,
      err_handler,
      open_handler,
      read_handler,
      complete_handler);
}


void HttpRequest::Put(
    const urdl::url &url,
    const HttpHeaderContentType &content_type,
    const std::string &post_content,
    const ErrorHandler &err_handler,
    const OpenHandler &open_handler,
    const ReadHandler &read_handler,
    const ResponseCompleteHandler &complete_handler) {
  Request(
      url,
      HttpRequestMethod::kPut,
      HttpHeaderContentType::kApplicationJson,
      post_content,
      err_handler,
      open_handler,
      read_handler,
      complete_handler);
}


void HttpRequest::Request(const urdl::url &url) {
  auto self{shared_from_this()};
  rstream_.async_open(
      url, [this, self](const boost::system::error_code &ec) {
    if (ec) {
      rstream_.close();
      ostream_close_handler_();
      err_handler_(ec);
      return;
    }
    open_handler_(rstream_.content_length());

    rstream_.async_read_some(boost::asio::buffer(buffer_),
                             std::bind(&HttpRequest::OnRead,
                                       self,
                                       std::placeholders::_1,
                                       std::placeholders::_2));
  });
}


void HttpRequest::OnRead(const boost::system::error_code &ec,
                         std::size_t length) {
  if (ec) {
    rstream_.close();
    ostream_close_handler_();
    complete_handler_();
    return;
  }
  read_handler_(length);

  out_->write(buffer_, length);
  rstream_.async_read_some(boost::asio::buffer(buffer_),
                           std::bind(&HttpRequest::OnRead,
                                     shared_from_this(),
                                     std::placeholders::_1,
                                     std::placeholders::_2));
}
}  // namespace ncstreamer
