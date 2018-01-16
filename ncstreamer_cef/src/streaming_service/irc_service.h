/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#ifndef NCSTREAMER_CEF_SRC_STREAMING_SERVICE_IRC_SERVICE_H_
#define NCSTREAMER_CEF_SRC_STREAMING_SERVICE_IRC_SERVICE_H_


#include <condition_variable> // NOLINT
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

  enum class ReadyStatus {
    kNone = 0,
    kConnecting,
    kCompleted,
  };

  enum class IoServiceRequest {
    kNone = 0,
    kRun,
    kBreak,
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
  void SendPongMessage();
  ReadyStatus GetReadyStatus();

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

  void SetReadyStatus(ReadyStatus ready_status);

  boost::asio::io_service io_service_;
  std::condition_variable io_service_cv_;
  IoServiceRequest io_service_request_;
  // This mutex is used for two purposes:
  // 1) to synchronize accesses to io_service_request_
  // 2) for the condition variable io_service_cv_
  mutable std::mutex io_service_mutex_;

  boost::asio::ssl::context ctx_;
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> stream_;
  boost::asio::streambuf streambuf_;

  std::string msg_;
  OnErrored error_;
  OnRead read_;
  std::thread thread_;

  mutable std::mutex ready_status_mutex_;
  ReadyStatus ready_status_;
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_STREAMING_SERVICE_IRC_SERVICE_H_
