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
      socket_(io_service_, ctx_),
      streambuf_{},
      msg_{},
      error_{},
      read_{},
      thread_{} {
  ctx_.load_verify_file("cacert.pem");
}


IrcService::~IrcService() {
  Close();
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

  boost::asio::ip::tcp::resolver resolver(io_service_);
  auto endpoint_iterator = resolver.resolve({host.c_str(), port.c_str()});

  boost::asio::async_connect(
      socket_.lowest_layer(),
      endpoint_iterator,
      boost::bind(&IrcService::HandleConnect, this,
          boost::asio::placeholders::error));

  thread_ = std::thread([this]() {io_service_.run();});
}


void IrcService::Close() {
  if (io_service_.stopped() == false) {
    io_service_.stop();
    socket_.lowest_layer().close();
  }
  if (thread_.joinable() == true)
    thread_.join();
}


void IrcService::HandleConnect(const boost::system::error_code &ec) {
  if (!ec) {
    socket_.async_handshake(
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
      socket_,
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
  } else {
    error_(ec);
  }
}


void IrcService::DoRead() {
  boost::asio::async_read_until(
      socket_, streambuf_, kIrcDelimiter,
      std::bind(&IrcService::ReadHandle, this, std::placeholders::_1,
          std::placeholders::_2));
}


void IrcService::ReadHandle(
    const boost::system::error_code &ec,
    const std::size_t &size) {
  std::string command{
      buffers_begin(streambuf_.data()),
      buffers_begin(streambuf_.data()) + size -
          std::string{kIrcDelimiter}.size()};

  streambuf_.consume(size);
  read_(command);

  DoRead();
}
}  // namespace ncstreamer
