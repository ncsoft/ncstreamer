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
      error_{},
      read_{},
      thread_{} {
  ctx_.load_verify_file("cacert.pem");
}


IrcService::~IrcService() {
  Close();
}


void IrcService::Connect(
    const std::string host,
    const std::string port,
    const std::string msg,
    const OnErrored &on_errored,
    const OnRead &on_read) {
  boost::asio::ip::tcp::resolver resolver(io_service_);
  auto endpoint_iterator = resolver.resolve({host.c_str(), port.c_str()});

  boost::asio::connect(socket_.lowest_layer(), endpoint_iterator);
  socket_.handshake(boost::asio::ssl::stream_base::client);

  boost::asio::write(socket_, boost::asio::buffer(msg.c_str(), msg.size()));

  error_ = on_errored;
  read_ = on_read;

  DoRead();

  thread_ = std::thread([this]() {io_service_.run();});
}


void IrcService::DoRead() {
  const std::string delimiter = "\r\n";

  boost::asio::async_read_until(
      socket_, streambuf_, delimiter,
      std::bind(&IrcService::ReadHandle, this, std::placeholders::_1,
          std::placeholders::_2));
}


void IrcService::Close() {
  if (io_service_.stopped() == false) {
    io_service_.stop();
    socket_.lowest_layer().close();
  }
  if (thread_.joinable() == true)
    thread_.join();
}


void IrcService::ReadHandle(
    const boost::system::error_code &ec,
    const std::size_t &size) {
  const std::string delimiter = "\r\n";

  std::string command{
      buffers_begin(streambuf_.data()),
      buffers_begin(streambuf_.data()) + size - delimiter.size()};

  streambuf_.consume(size);
  DoRead();
}
}  // namespace ncstreamer
