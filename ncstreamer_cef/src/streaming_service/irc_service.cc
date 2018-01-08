/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#include "ncstreamer_cef/src/streaming_service/irc_service.h"

#include <algorithm>

#include "boost/bind.hpp"


namespace ncstreamer {
IrcService::IrcService()
    : io_service_{},
      ctx_(boost::asio::ssl::context::sslv23),
      stream_(io_service_, ctx_),
      io_service_cv_{},
      io_service_request_{IoServiceRequest::kNone},
      io_service_mutex_{},
      streambuf_{},
      msg_{},
      error_{},
      read_{},
      thread_{},
      ready_status_mutex_{},
      ready_status_{IrcService::ReadyStatus::kNone} {
  ctx_.load_verify_file("cacert.pem");

  thread_ = std::thread([this]() {
    for (;;) {
      std::unique_lock<std::mutex> lk(io_service_mutex_);
      io_service_cv_.wait(lk);

      if (io_service_request_ == IoServiceRequest::kRun) {
        io_service_.reset();
        io_service_.run();
      } else if (io_service_request_ == IoServiceRequest::kBreak) {
        break;
      }
    }
  });
}


IrcService::~IrcService() {
  Close();

  {
    std::lock_guard<std::mutex> lk(io_service_mutex_);
    io_service_request_ = IoServiceRequest::kBreak;
  }
  io_service_cv_.notify_all();

  if (thread_.joinable() == true) {
    thread_.join();
  }
}


static const char *kIrcDelimiter{[]() {
  static const char *kDelimiter{"\r\n"};
  return kDelimiter;
}()};


void IrcService::Connect(
    const std::string host,
    const std::string port,
    const std::string msg,
    const OnErrored &on_errored,
    const OnRead &on_read) {
  msg_ = msg;
  error_ = on_errored;
  read_ = on_read;

  SetReadyStatus(IrcService::ReadyStatus::kConnecting);

  boost::asio::ip::tcp::resolver resolver(io_service_);
  auto endpoint_iterator = resolver.resolve({host.c_str(), port.c_str()});

  boost::asio::async_connect(
      stream_.lowest_layer(),
      endpoint_iterator,
      boost::bind(&IrcService::HandleConnect, this,
          boost::asio::placeholders::error));

  {
    std::lock_guard<std::mutex> lk(io_service_mutex_);
    io_service_request_ = IoServiceRequest::kRun;
  }
  io_service_cv_.notify_all();
}


void IrcService::Close() {
  io_service_.stop();
  SetReadyStatus(IrcService::ReadyStatus::kNone);
}


void IrcService::HandleConnect(const boost::system::error_code &ec) {
  if (!ec) {
    stream_.async_handshake(
        boost::asio::ssl::stream_base::client,
        boost::bind(&IrcService::HandleHandshake, this,
            boost::asio::placeholders::error));
  } else {
    error_(ec);
  }
}


void IrcService::HandleHandshake(const boost::system::error_code &ec) {
  if (!ec) {
    boost::asio::async_write(
      stream_,
      boost::asio::buffer(msg_.c_str(), msg_.size()),
      boost::bind(&IrcService::HandleWrite, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  } else {
    error_(ec);
  }
}


void IrcService::HandleWrite(
    const boost::system::error_code &ec,
    const std::size_t &size) {
  if (!ec) {
    DoRead();
    SetReadyStatus(IrcService::ReadyStatus::kCompleted);
  } else {
    error_(ec);
  }
}


void IrcService::DoRead() {
  boost::asio::async_read_until(
      stream_, streambuf_, kIrcDelimiter,
      std::bind(&IrcService::ReadHandle, this, std::placeholders::_1,
          std::placeholders::_2));
}


void IrcService::ReadHandle(
    const boost::system::error_code &ec,
    const std::size_t &size) {
  if (!ec) {
    std::string command{
        buffers_begin(streambuf_.data()),
        buffers_begin(streambuf_.data()) + size -
            std::string{kIrcDelimiter}.size()};

    streambuf_.consume(size);
    read_(command);

    DoRead();
  } else {
    error_(ec);
  }
}


IrcService::ReadyStatus IrcService::GetReadyStatus() {
  IrcService::ReadyStatus ready_status{IrcService::ReadyStatus::kNone};
  {
    std::lock_guard<std::mutex> lock{ready_status_mutex_};
    ready_status = ready_status_;
  }
  return ready_status;
}


void IrcService::SetReadyStatus(IrcService::ReadyStatus ready_status) {
  std::lock_guard<std::mutex> lock{ready_status_mutex_};
  ready_status_ = ready_status;
}
}  // namespace ncstreamer
