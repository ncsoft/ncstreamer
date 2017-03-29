/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/lib/http_download_service.h"


namespace ncstreamer {
HttpDownloadService::HttpDownloadService()
    : io_service_{},
      io_service_work_{io_service_},
      io_thread_{[this]() {
        io_service_.run();
      }},
      downloader_{new HttpDownloader{&io_service_}} {
}


HttpDownloadService::~HttpDownloadService() {
  io_service_.stop();
  if (io_thread_.joinable() == true) {
    io_thread_.join();
  }
}


void HttpDownloadService::DownloadAsString(
    const std::string &uri,
    const urdl::http::request_method &method,
    const HttpDownloader::ErrorHandler &err_handler,
    const HttpDownloader::OpenHandler &open_handler,
    const HttpDownloader::ReadHandler &read_handler,
    const HttpDownloader::CompleteHandlerAsString &complete_handler) {
  downloader_->DownloadAsString(
      uri,
      method,
      err_handler,
      open_handler,
      read_handler,
      complete_handler);
}


void HttpDownloadService::DownloadAsString(
    const std::string &uri,
    const urdl::http::request_method &method,
    const HttpDownloader::ErrorHandler &err_handler,
    const HttpDownloader::CompleteHandlerAsString &complete_handler) {
  static const HttpDownloader::OpenHandler kDefaultOpenHandler{
        [](std::size_t /*file_size*/) {}};
  static const HttpDownloader::ReadHandler kDefaultReadHandler{
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
