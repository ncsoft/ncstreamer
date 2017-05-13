/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/remote_server.h"

#include <cassert>


namespace {
namespace placeholders = websocketpp::lib::placeholders;
}  // unnamed namespace


namespace ncstreamer {
void RemoteServer::SetUp(
    const BrowserApp *browser_app,
    uint16_t port) {
  assert(!static_instance);
  static_instance = new RemoteServer{
      browser_app,
      port};
}


void RemoteServer::ShutDown() {
  assert(static_instance);
  delete static_instance;
  static_instance = nullptr;
}


RemoteServer *RemoteServer::Get() {
  assert(static_instance);
  return static_instance;
}


RemoteServer::RequestCache::RequestCache()
    : mutex_{},
      cache_{},
      last_key_{0} {
}


RemoteServer::RequestCache::~RequestCache() {
}


int RemoteServer::RequestCache::CheckIn(
    ws::connection_hdl connection) {
  int last_key{0};
  {
    std::lock_guard<std::mutex> lock{mutex_};

    ++last_key_;
    cache_.emplace(last_key_, connection);
    last_key = last_key_;
  }
  return last_key;
}


ws::connection_hdl RemoteServer::RequestCache::CheckOut(
    int key) {
  ws::connection_hdl connection{};
  {
    std::lock_guard<std::mutex> lock{mutex_};

    auto i = cache_.find(key);
    if (i != cache_.end()) {
      connection = i->second;
      cache_.erase(i);
    }
  }
  return connection;
}


RemoteServer::RemoteServer(
    const BrowserApp *browser_app,
    uint16_t port)
    : browser_app_{browser_app},
      io_service_{},
      io_service_work_{io_service_},
      server_{},
      server_threads_{},
      server_log_{},
      request_cache_{} {
  server_log_.open("remote_server.log");
  server_.set_access_channels(ws::log::alevel::all);
  server_.set_access_channels(ws::log::elevel::all);
  server_.get_alog().set_ostream(&server_log_);
  server_.get_elog().set_ostream(&server_log_);

  ws::lib::error_code ec;
  server_.init_asio(&io_service_, ec);
  if (ec) {
    // TODO(khpark): log error.
    assert(false);
    return;
  }

  server_.set_fail_handler(ws::lib::bind(
      &RemoteServer::OnFail, this, placeholders::_1));
  server_.set_open_handler(ws::lib::bind(
      &RemoteServer::OnOpen, this, placeholders::_1));
  server_.set_close_handler(ws::lib::bind(
      &RemoteServer::OnClose, this, placeholders::_1));
  server_.set_message_handler(ws::lib::bind(
      &RemoteServer::OnMessage, this, placeholders::_1, placeholders::_2));

  server_.listen(port);
  server_.start_accept();

  static const std::size_t kServerThreadsSize{1};  // just one enough.
  for (std::size_t i = 0; i < kServerThreadsSize; ++i) {
    server_threads_.emplace_back([this]() {
      server_.run();
    });
  }
}


RemoteServer::~RemoteServer() {
  server_.stop_listening();
  server_.stop();
  for (auto &t : server_threads_) {
    if (t.joinable() == true) {
      t.join();
    }
  }
}


void RemoteServer::OnFail(ws::connection_hdl connection) {
  // TBD: log error.
}


void RemoteServer::OnOpen(ws::connection_hdl connection) {
}


void RemoteServer::OnClose(ws::connection_hdl connection) {
}


void RemoteServer::OnMessage(ws::connection_hdl connection,
                             ws::connection<Asio>::message_ptr msg) {
}


RemoteServer *RemoteServer::static_instance{nullptr};
}  // namespace ncstreamer
