/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_LIB_HTTP_REQUEST_H_
#define SRC_LIB_HTTP_REQUEST_H_


#include <functional>
#include <memory>
#include <ostream>
#include <string>

#include "boost/asio/io_service.hpp"

#pragma warning(push)
#pragma warning(disable: 4244)
#include "urdl/istream.hpp"
#pragma warning(pop)


namespace ncstreamer {
class HttpRequest
    : public std::enable_shared_from_this<HttpRequest> {
 public:
  using ErrorHandler = std::function<void(const boost::system::error_code &ec)>;
  using OpenHandler = std::function<void(std::size_t file_size)>;
  using ReadHandler = std::function<void(std::size_t read_size)>;
  using DownloadCompleteHandler = std::function<void()>;
  using CompleteHandlerAsString = std::function<void(const std::string &data)>;

  explicit HttpRequest(boost::asio::io_service *svc);

  void Download(
      const urdl::url &url,
      const std::string &file_name,
      const ErrorHandler &err_handler,
      const OpenHandler &open_handler,
      const ReadHandler &read_handler,
      const DownloadCompleteHandler &complete_handler);

  void Request(
      const urdl::url &url,
      const urdl::http::request_method &method,
      const ErrorHandler &err_handler,
      const OpenHandler &open_handler,
      const ReadHandler &read_handler,
      const CompleteHandlerAsString &complete_handler);

 private:
  using CompleteHandler = std::function<void()>;
  using OstreamCloseHandler = std::function<void()>;

  void Request(const urdl::url &url);

  void OnRead(const boost::system::error_code &ec,
              std::size_t length);

  urdl::read_stream rstream_;
  std::unique_ptr<std::ostream> out_;
  char buffer_[1024];

  ErrorHandler err_handler_;
  OpenHandler open_handler_;
  ReadHandler read_handler_;
  CompleteHandler complete_handler_;

  OstreamCloseHandler ostream_close_handler_;
};
}  // namespace ncstreamer


#endif  // SRC_LIB_HTTP_REQUEST_H_
