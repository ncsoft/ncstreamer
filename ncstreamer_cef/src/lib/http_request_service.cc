/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/lib/http_request_service.h"


namespace ncstreamer {
HttpRequestService::HttpRequestService()
    : io_service_{},
      io_service_work_{io_service_},
      io_thread_{[this]() {
        io_service_.run();
      }},
      http_request_{new HttpRequest{&io_service_}} {
}


HttpRequestService::~HttpRequestService() {
  io_service_.stop();
  if (io_thread_.joinable() == true) {
    io_thread_.join();
  }
}


void HttpRequestService::Get(
    const std::string &uri,
    const HttpRequest::ErrorHandler &err_handler,
    const HttpRequest::OpenHandler &open_handler,
    const HttpRequest::ReadHandler &read_handler,
    const HttpRequest::ResponseCompleteHandler &complete_handler) {
  http_request_->Get(
      uri,
      err_handler,
      open_handler,
      read_handler,
      complete_handler);
}


void HttpRequestService::Get(
    const std::string &uri,
    const HttpRequest::ErrorHandler &err_handler,
    const HttpRequest::ResponseCompleteHandler &complete_handler) {
  static const HttpRequest::OpenHandler kDefaultOpenHandler{
        [](std::size_t /*file_size*/) {}};
  static const HttpRequest::ReadHandler kDefaultReadHandler{
        [](std::size_t /*read_size*/) {}};

  Get(
      uri,
      err_handler,
      kDefaultOpenHandler,
      kDefaultReadHandler,
      complete_handler);
}


void HttpRequestService::Post(
    const std::string &uri,
    const boost::property_tree::ptree &post_content,
    const HttpRequest::ErrorHandler &err_handler,
    const HttpRequest::OpenHandler &open_handler,
    const HttpRequest::ReadHandler &read_handler,
    const HttpRequest::ResponseCompleteHandler &complete_handler) {
  http_request_->Post(
      uri,
      post_content,
      err_handler,
      open_handler,
      read_handler,
      complete_handler);
}


void HttpRequestService::Post(
    const std::string &uri,
    const boost::property_tree::ptree &post_content,
    const HttpRequest::ErrorHandler &err_handler,
    const HttpRequest::ResponseCompleteHandler &complete_handler) {
  static const HttpRequest::OpenHandler kDefaultOpenHandler{
        [](std::size_t /*file_size*/) {}};
  static const HttpRequest::ReadHandler kDefaultReadHandler{
        [](std::size_t /*read_size*/) {}};

  Post(
      uri,
      post_content,
      err_handler,
      kDefaultOpenHandler,
      kDefaultReadHandler,
      complete_handler);
}
}  // namespace ncstreamer
