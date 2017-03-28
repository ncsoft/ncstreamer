/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_LIB_HTTP_DOWNLOAD_SERVICE_H_
#define SRC_LIB_HTTP_DOWNLOAD_SERVICE_H_


#include <memory>
#include <string>
#include <thread>  // NOLINT

#include "boost/asio/io_service.hpp"

#include "src/lib/http_downloader.h"


namespace ncstreamer {
class HttpDownloadService {
 public:
  HttpDownloadService();
  virtual ~HttpDownloadService();

  void DownloadAsString(
      const std::string &uri,
      const HttpDownloader::ErrorHandler &err_handler,
      const HttpDownloader::OpenHandler &open_handler,
      const HttpDownloader::ReadHandler &read_handler,
      const HttpDownloader::CompleteHandlerAsString &complete_handler);

  void DownloadAsString(
      const std::string &uri,
      const HttpDownloader::ErrorHandler &err_handler,
      const HttpDownloader::CompleteHandlerAsString &complete_handler);

 private:
  boost::asio::io_service io_service_;
  boost::asio::io_service::work io_service_work_;
  std::thread io_thread_;

  std::shared_ptr<HttpDownloader> downloader_;
};
}  // namespace ncstreamer


#endif  // SRC_LIB_HTTP_DOWNLOAD_SERVICE_H_
