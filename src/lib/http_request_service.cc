/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/lib/http_request_service.h"


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


void HttpRequestService::DownloadAsString(
    const std::string &uri,
    const urdl::http::request_method &method,
    const HttpRequest::ErrorHandler &err_handler,
    const HttpRequest::OpenHandler &open_handler,
    const HttpRequest::ReadHandler &read_handler,
    const HttpRequest::CompleteHandlerAsString &complete_handler) {
  http_request_->DownloadAsString(
      uri,
      method,
      err_handler,
      open_handler,
      read_handler,
      complete_handler);
}


void HttpRequestService::DownloadAsString(
    const std::string &uri,
    const urdl::http::request_method &method,
    const HttpRequest::ErrorHandler &err_handler,
    const HttpRequest::CompleteHandlerAsString &complete_handler) {
  static const HttpRequest::OpenHandler kDefaultOpenHandler{
        [](std::size_t /*file_size*/) {}};
  static const HttpRequest::ReadHandler kDefaultReadHandler{
        [](std::size_t /*read_size*/) {}};

  DownloadAsString(
      uri,
      method,
      err_handler,
      kDefaultOpenHandler,
      kDefaultReadHandler,
      complete_handler);
}
}  // namespace ncstreamer
