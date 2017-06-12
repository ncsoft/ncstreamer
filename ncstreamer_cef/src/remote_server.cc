/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/remote_server.h"

#include <cassert>
#include <functional>
#include <sstream>
#include <unordered_map>

#include "boost/property_tree/json_parser.hpp"

#include "ncstreamer_cef/src/js_executor.h"
#include "ncstreamer_cef/src/remote_message_types.h"


namespace {
namespace placeholders = websocketpp::lib::placeholders;
}  // unnamed namespace


namespace ncstreamer {
void RemoteServer::SetUp(
    const BrowserApp *browser_app) {
  assert(!static_instance);
  static_instance = new RemoteServer{browser_app};
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
    const std::string &source_title,
    const std::string &user_name,
    const std::string &quality) {
  websocketpp::connection_hdl connection = request_cache_.CheckOut(request_key);
  if (!connection.lock()) {
    LogWarning("RespondStreamingStatus: !connection.lock()");
    return;
  }

  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        RemoteMessage::MessageType::kStreamingStatusResponse));
    tree.put("status", status);
    tree.put("sourceTitle", source_title);
    tree.put("userName", user_name);
    tree.put("quality", quality);
    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  server_.send(connection, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    LogError(ec.message());
    return;
  }
}


void RemoteServer::RespondStreamingStart(
    int request_key,
    const std::string &error,
    const std::string &source,
    const std::string &user_page,
    const std::string &privacy,
    const std::string &description,
    const std::string &mic,
    const std::string &service_provider,
    const std::string &stream_url) {
  websocketpp::connection_hdl connection = request_cache_.CheckOut(request_key);
  if (!connection.lock()) {
    LogWarning("RespondStreamingStart: !connection.lock()");
    return;
  }

  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        RemoteMessage::MessageType::kStreamingStartResponse));
    tree.put("error", error);
    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  server_.send(connection, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    LogError(ec.message());
    return;
  }

  if (error.empty() == true) {
    BroadcastStreamingStart(
        source,
        user_page,
        privacy,
        description,
        mic,
        service_provider,
        stream_url);
  }
}


void RemoteServer::RespondStreamingStop(
    int request_key,
    const std::string &error,
    const std::string &source) {
  websocketpp::connection_hdl connection = request_cache_.CheckOut(request_key);
  if (!connection.lock()) {
    LogWarning("RespondStreamingStop: !connection.lock()");
    return;
  }

  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        RemoteMessage::MessageType::kStreamingStopResponse));
    tree.put("error", error);
    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  server_.send(connection, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    LogError(ec.message());
    return;
  }

  if (error.empty() == true) {
    BroadcastStreamingStop(source);
  }
}


void RemoteServer::RespondSettingsQualityUpdate(
    int request_key,
    const std::string &error) {
  websocketpp::connection_hdl connection = request_cache_.CheckOut(request_key);
  if (!connection.lock()) {
    LogWarning("RespondSettingsQualityUpdate: !connection.lock()");
    return;
  }

  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        RemoteMessage::MessageType::kSettingsQualityUpdateResponse));
    tree.put("error", error);
    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  server_.send(connection, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    LogError(ec.message());
    return;
  }
}


std::size_t RemoteServer::ConnectionHasher::operator()(
    const websocketpp::connection_hdl &connection) const {
  return reinterpret_cast<std::size_t>(connection.lock().get());
}


bool RemoteServer::ConnectionKeyeq::operator()(
    const websocketpp::connection_hdl &left,
    const websocketpp::connection_hdl &right) const {
  return (left.lock().get() == right.lock().get());
}


RemoteServer::RequestCache::RequestCache()
    : mutex_{},
      cache_{},
      last_key_{0} {
}


RemoteServer::RequestCache::~RequestCache() {
}


int RemoteServer::RequestCache::CheckIn(
    websocketpp::connection_hdl connection) {
  int last_key{0};
  {
    std::lock_guard<std::mutex> lock{mutex_};

    ++last_key_;
    cache_.emplace(last_key_, connection);
    last_key = last_key_;
  }
  return last_key;
}


websocketpp::connection_hdl RemoteServer::RequestCache::CheckOut(
    int key) {
  websocketpp::connection_hdl connection{};
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
    const BrowserApp *browser_app)
    : browser_app_{browser_app},
      io_service_{},
      io_service_work_{io_service_},
      server_{},
      server_threads_{},
      server_log_{},
      connections_{},
      request_cache_{} {
}


bool RemoteServer::Start(uint16_t port) {
  server_log_.open("remote_server.log");
  server_.set_access_channels(websocketpp::log::alevel::all);
  server_.set_access_channels(websocketpp::log::elevel::all);
  server_.get_alog().set_ostream(&server_log_);
  server_.get_elog().set_ostream(&server_log_);

  {
    websocketpp::lib::error_code ec;
    server_.init_asio(&io_service_, ec);
    if (ec) {
      LogError(ec.message());
      return false;
    }
  }

  server_.set_fail_handler(websocketpp::lib::bind(
      &RemoteServer::OnFail, this, placeholders::_1));
  server_.set_open_handler(websocketpp::lib::bind(
      &RemoteServer::OnOpen, this, placeholders::_1));
  server_.set_close_handler(websocketpp::lib::bind(
      &RemoteServer::OnClose, this, placeholders::_1));
  server_.set_message_handler(websocketpp::lib::bind(
      &RemoteServer::OnMessage, this, placeholders::_1, placeholders::_2));

  {
    websocketpp::lib::error_code ec;
    server_.listen({boost::asio::ip::address::from_string("::1"), port}, ec);
    if (ec) {
      LogError(ec.message());
      return false;
    }
  }
  server_.start_accept();

  static const std::size_t kServerThreadsSize{1};  // just one enough.
  for (std::size_t i = 0; i < kServerThreadsSize; ++i) {
    server_threads_.emplace_back([this]() {
      server_.run();
    });
  }

  return true;
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


void RemoteServer::OnFail(websocketpp::connection_hdl connection) {
  LogError("OnFail");

  connections_.erase(connection);
}


void RemoteServer::OnOpen(websocketpp::connection_hdl connection) {
  LogInfo("OnOpen");

  connections_.emplace(connection);
}


void RemoteServer::OnClose(websocketpp::connection_hdl connection) {
  LogInfo("OnClose");

  connections_.erase(connection);
}


void RemoteServer::OnMessage(
    websocketpp::connection_hdl connection,
    websocketpp::connection<websocketpp::config::asio>::message_ptr msg) {
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
      const websocketpp::connection_hdl &,
      const boost::property_tree::ptree &/*msg*/)>;
  static const std::unordered_map<RemoteMessage::MessageType,
                                  MessageHandler> kMessageHandlers{
      {RemoteMessage::MessageType::kStreamingStatusRequest,
       std::bind(&RemoteServer::OnStreamingStatusRequest,
           this, std::placeholders::_1, std::placeholders::_2)},
      {RemoteMessage::MessageType::kStreamingStartRequest,
       std::bind(&RemoteServer::OnStreamingStartRequest,
           this, std::placeholders::_1, std::placeholders::_2)},
      {RemoteMessage::MessageType::kStreamingStopRequest,
       std::bind(&RemoteServer::OnStreamingStopRequest,
           this, std::placeholders::_1, std::placeholders::_2)},
      {RemoteMessage::MessageType::kSettingsQualityUpdateRequest,
       std::bind(&RemoteServer::OnSettingsQualityUpdateRequest,
           this, std::placeholders::_1, std::placeholders::_2)},
      {RemoteMessage::MessageType::kNcStreamerExitRequest,
       std::bind(&RemoteServer::OnNcStreamerExitRequest,
           this, std::placeholders::_1, std::placeholders::_2)}};

  auto i = kMessageHandlers.find(msg_type);
  if (i == kMessageHandlers.end()) {
    std::stringstream err;
    err << "unknown message type: " << static_cast<int>(msg_type);
    LogError(err.str());
    return;
  }
  i->second(connection, msg_tree);
}


void RemoteServer::OnStreamingStatusRequest(
    const websocketpp::connection_hdl &connection,
    const boost::property_tree::ptree &/*tree*/) {
  int request_key = request_cache_.CheckIn(connection);

  JsExecutor::Execute(
      browser_app_->GetMainBrowser(),
      "remote.onStreamingStatusRequest",
      request_key);
}


void RemoteServer::OnStreamingStartRequest(
    const websocketpp::connection_hdl &connection,
    const boost::property_tree::ptree &tree) {
  const std::string &title = tree.get("title", "");
  if (title.empty()) {
    LogError("OnStreamingStartRequest: title empty.");
    return;
  }

  boost::property_tree::ptree args;
  args.add("sourceTitle", title);

  int request_key = request_cache_.CheckIn(connection);

  JsExecutor::Execute(
      browser_app_->GetMainBrowser(),
      "remote.onStreamingStartRequest",
      request_key,
      args);
}


void RemoteServer::OnStreamingStopRequest(
    const websocketpp::connection_hdl &connection,
    const boost::property_tree::ptree &tree) {
  const std::string &title = tree.get("title", "");
  if (title.empty()) {
    LogError("OnStreamingStopRequest: title empty.");
    return;
  }

  boost::property_tree::ptree args;
  args.add("sourceTitle", title);

  int request_key = request_cache_.CheckIn(connection);

  JsExecutor::Execute(
      browser_app_->GetMainBrowser(),
      "remote.onStreamingStopRequest",
      request_key,
      args);
}


void RemoteServer::OnSettingsQualityUpdateRequest(
    const websocketpp::connection_hdl &connection,
    const boost::property_tree::ptree &tree) {
  const std::string &quality = tree.get("quality", "");
  if (quality.empty()) {
    LogError("OnSettingsQualityUpdateRequest: quality empty.");
    return;
  }

  boost::property_tree::ptree args;
  args.add("quality", quality);

  int request_key = request_cache_.CheckIn(connection);

  JsExecutor::Execute(
      browser_app_->GetMainBrowser(),
      "remote.onSettingsQualityUpdateRequest",
      request_key,
      args);
}


void RemoteServer::OnNcStreamerExitRequest(
    const websocketpp::connection_hdl &/*connection*/,
    const boost::property_tree::ptree &/*tree*/) {
  HWND wnd = browser_app_->GetMainBrowser()->GetHost()->GetWindowHandle();
  ::PostMessage(wnd, WM_CLOSE, NULL, NULL);
}


void RemoteServer::BroadcastStreamingStart(
    const std::string &source,
    const std::string &user_page,
    const std::string &privacy,
    const std::string &description,
    const std::string &mic,
    const std::string &service_provider,
    const std::string &stream_url) {
  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        RemoteMessage::MessageType::kStreamingStartEvent));
    tree.put("source", source);
    tree.put("userPage", user_page);
    tree.put("privacy", privacy);
    tree.put("description", description);
    tree.put("mic", mic);
    tree.put("serviceProvider", service_provider);
    tree.put("streamUrl", stream_url);
    boost::property_tree::write_json(msg, tree, false);
  }
  Broadcast(msg.str());
}


void RemoteServer::BroadcastStreamingStop(
    const std::string &source) {
  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        RemoteMessage::MessageType::kStreamingStopEvent));
    tree.put("source", source);
    boost::property_tree::write_json(msg, tree, false);
  }
  Broadcast(msg.str());
}


void RemoteServer::Broadcast(const std::string &msg) {
  for (const auto &connection : connections_) {
    websocketpp::lib::error_code ec;
    server_.send(connection, msg, websocketpp::frame::opcode::text, ec);
    if (ec) {
      LogError(ec.message());
    }
  }
}


void RemoteServer::LogError(const std::string &err_msg) {
  server_.get_elog().write(websocketpp::log::elevel::rerror, err_msg);
}


void RemoteServer::LogWarning(const std::string &warn_msg) {
  server_.get_elog().write(websocketpp::log::elevel::warn, warn_msg);
}


void RemoteServer::LogInfo(const std::string &info_msg) {
  server_.get_elog().write(websocketpp::log::elevel::info, info_msg);
}


RemoteServer *RemoteServer::static_instance{nullptr};
}  // namespace ncstreamer
