/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#ifndef NCSTREAMER_CEF_SRC_STREAMING_SERVICE_IRC_SERVICE_H_
#define NCSTREAMER_CEF_SRC_STREAMING_SERVICE_IRC_SERVICE_H_


#include <mutex>  // NOLINT
#include <string>
#include <thread>  // NOLINT

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"


namespace ncstreamer {
class IrcService {
 public:
  using OnErrored = std::function<void(const boost::system::error_code &ec)>;
  using OnRead = std::function<void(const std::string &msg)>;

  enum class ReadyType {
    kNone = 0,
    kConnecting,
    kCompleted,
  };

  IrcService();
  virtual ~IrcService();

  void Connect(
      const std::string host,
      const std::string port,
      const std::string msg,
      const OnErrored &on_errored,
      const OnRead &on_read);
  void Close();
  ReadyType GetReadyType();

 private:
  void HandleConnect(const boost::system::error_code &ec);
  void HandleHandshake(const boost::system::error_code &ec);
  void HandleWrite(
      const boost::system::error_code &ec,
      const std::size_t &size);

  void DoRead();
  void ReadHandle(
      const boost::system::error_code &ec,
      const std::size_t &size);

  void SetReadyType(ReadyType ready_type);

  boost::asio::io_service io_service_;
  boost::asio::ssl::context ctx_;
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
  boost::asio::streambuf streambuf_;
  std::string msg_;
  OnErrored error_;
  OnRead read_;
  std::thread thread_;

  mutable std::mutex ready_type_mutex_;
  ReadyType ready_type_;
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_STREAMING_SERVICE_IRC_SERVICE_H_
