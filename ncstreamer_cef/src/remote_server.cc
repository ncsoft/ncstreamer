/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/remote_server.h"

#include <cassert>
#include <functional>
#include <unordered_map>

#include "boost/property_tree/json_parser.hpp"

#include "ncstreamer_cef/src/js_executor.h"
#include "ncstreamer_cef/src/remote_message_types.h"


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


void RemoteServer::RespondStreamingStatus(
    int request_key,
    const std::string &status,
    const std::string &source_title) {
  ws::connection_hdl connection = request_cache_.CheckOut(request_key);
  if (!connection.lock()) {
    // TODO(khpark): log warning.
    return;
  }

  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        RemoteMessage::MessageType::kStreamingStatusResponse));
    tree.put("status", status);
    tree.put("source_title", source_title);
    boost::property_tree::write_json(msg, tree, false);
  }

  ws::lib::error_code ec;
  server_.send(connection, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    // TODO(khpark): log error.
    return;
  }
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
  boost::property_tree::ptree msg_tree;
  auto msg_type{RemoteMessage::MessageType::kUndefined};

  std::stringstream ss{msg->get_payload()};
  try {
    boost::property_tree::read_json(ss, msg_tree);
    msg_type = static_cast<RemoteMessage::MessageType>(
        msg_tree.get<int>("type"));
  } catch (const std::exception &/*e*/) {
    msg_type = RemoteMessage::MessageType::kUndefined;
  }

  using MessageHandler = std::function<void(
      const ws::connection_hdl &,
      const boost::property_tree::ptree &/*msg*/)>;
  static const std::unordered_map<RemoteMessage::MessageType,
                                  MessageHandler> kMessageHandlers{
      {RemoteMessage::MessageType::kStreamingStatusRequest,
       std::bind(&RemoteServer::OnStreamingStatusRequest,
           this, std::placeholders::_1, std::placeholders::_2)}};

  auto i = kMessageHandlers.find(msg_type);
  if (i == kMessageHandlers.end()) {
    // TBD
    assert(false);
    return;
  }
  i->second(connection, msg_tree);
}


void RemoteServer::OnStreamingStatusRequest(
    const ws::connection_hdl &connection,
    const boost::property_tree::ptree &/*tree*/) {
  int request_key = request_cache_.CheckIn(connection);

  JsExecutor::Execute(
      browser_app_->GetMainBrowser(),
      "remote.onStreamingStatusRequest",
      request_key);
}


RemoteServer *RemoteServer::static_instance{nullptr};
}  // namespace ncstreamer
