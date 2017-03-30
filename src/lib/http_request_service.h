/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_LIB_HTTP_REQUEST_SERVICE_H_
#define SRC_LIB_HTTP_REQUEST_SERVICE_H_


#include <memory>
#include <string>
#include <thread>  // NOLINT

#include "boost/asio/io_service.hpp"

#include "src/lib/http_request.h"


namespace ncstreamer {
class HttpRequestService {
 public:
  HttpRequestService();
  virtual ~HttpRequestService();

  void Request(
      const std::string &uri,
      const urdl::http::request_method &method,
      const HttpRequest::ErrorHandler &err_handler,
      const HttpRequest::OpenHandler &open_handler,
      const HttpRequest::ReadHandler &read_handler,
      const HttpRequest::CompleteHandlerAsString &complete_handler);

  void Request(
      const std::string &uri,
      const urdl::http::request_method &method,
      const HttpRequest::ErrorHandler &err_handler,
      const HttpRequest::CompleteHandlerAsString &complete_handler);

 private:
  boost::asio::io_service io_service_;
  boost::asio::io_service::work io_service_work_;
  std::thread io_thread_;

  std::shared_ptr<HttpRequest> http_request_;
};
}  // namespace ncstreamer


#endif  // SRC_LIB_HTTP_REQUEST_SERVICE_H_
