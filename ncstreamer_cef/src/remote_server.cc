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


void RemoteServer::ResponseComments(
    int request_key,
    const std::string &error,
    const std::string &comments) {
  websocketpp::connection_hdl connection = request_cache_.CheckOut(request_key);
  if (!connection.lock()) {
    LogWarning("ResponseChatMessage: !connection.lock()");
    return;
  }

  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        RemoteMessage::MessageType::kStreamingCommentsResponse));
    tree.put("error", error);
    tree.put("comments", comments);
    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  server_.send(connection, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    LogError(ec.message());
    return;
  }
}


void RemoteServer::ResponseViewers(
    int request_key,
    const std::string &error,
    const std::string &viewers) {
  websocketpp::connection_hdl connection = request_cache_.CheckOut(request_key);
  if (!connection.lock()) {
    LogWarning("ResponseViewersMessage: !connection.lock()");
    return;
  }

  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        RemoteMessage::MessageType::kStreamingViewersResponse));
    tree.put("error", error);
    tree.put("viewers", viewers);
    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  server_.send(connection, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    LogError(ec.message());
    return;
  }
}


void RemoteServer::NotifyStreamingStart(
    int request_key,
    const std::string &error,
    const std::string &source,
    const std::string &user_page,
    const std::string &privacy,
    const std::string &description,
    const std::string &mic,
    const std::string &service_provider,
    const std::string &stream_url,
    const std::string &post_url) {
  if (request_key != 0) {
    RespondStreamingStart(request_key, error);
  }

  if (error.empty() == true) {
    BroadcastStreamingStart(
        source,
        user_page,
        privacy,
        description,
        mic,
        service_provider,
        stream_url,
        post_url);
  }
}


void RemoteServer::NotifyStreamingStop(
    int request_key,
    const std::string &error,
    const std::string &source) {
  if (request_key != 0) {
    RespondStreamingStop(request_key, error);
  }

  if (error.empty() == true) {
    BroadcastStreamingStop(source);
  }
}


void RemoteServer::NotifySettingsQualityUpdate(
    int request_key,
    const std::string &error) {
  if (request_key != 0) {
    RespondSettingsQualityUpdate(request_key, error);
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
      {RemoteMessage::MessageType::kStreamingCommentsRequest,
       std::bind(&RemoteServer::OnCommentsRequest,
           this, std::placeholders::_1, std::placeholders::_2)},
      {RemoteMessage::MessageType::kStreamingViewersRequest,
       std::bind(&RemoteServer::OnViewersRequest,
           this, std::placeholders::_1, std::placeholders::_2) },
      {RemoteMessage::MessageType::kSettingsWebcamSearchRequest,
       std::bind(&RemoteServer::OnSettingsWebcamSearchRequest,
           this, std::placeholders::_1, std::placeholders::_2)},
      {RemoteMessage::MessageType::kSettingsWebcamOnRequest,
       std::bind(&RemoteServer::OnSettingsWebcamOnRequest,
           this, std::placeholders::_1, std::placeholders::_2)},
      {RemoteMessage::MessageType::kSettingsWebcamOffRequest,
       std::bind(&RemoteServer::OnSettingsWebcamOffRequest,
           this, std::placeholders::_1, std::placeholders::_2)},
      {RemoteMessage::MessageType::kSettingsWebcamSizeRequest,
       std::bind(&RemoteServer::OnSettingsWebcamSizeRequest,
           this, std::placeholders::_1, std::placeholders::_2)},
      {RemoteMessage::MessageType::kSettingsWebcamPositionRequest,
       std::bind(&RemoteServer::OnSettingsWebcamPositionRequest,
           this, std::placeholders::_1, std::placeholders::_2)},
      {RemoteMessage::MessageType::kSettingsChromaKeyOnRequest,
       std::bind(&RemoteServer::OnSettingsChromaKeyOnRequest,
           this, std::placeholders::_1, std::placeholders::_2)},
      {RemoteMessage::MessageType::kSettingsChromaKeyOffRequest,
       std::bind(&RemoteServer::OnSettingsChromaKeyOffRequest,
           this, std::placeholders::_1, std::placeholders::_2)},
      {RemoteMessage::MessageType::kSettingsChromaKeyColorRequest,
       std::bind(&RemoteServer::OnSettingsChromaKeyColorRequest,
           this, std::placeholders::_1, std::placeholders::_2)},
      {RemoteMessage::MessageType::kSettingsChromaKeySimilarityRequest,
       std::bind(&RemoteServer::OnSettingsChromaKeySimilarityRequest,
           this, std::placeholders::_1, std::placeholders::_2)},
      {RemoteMessage::MessageType::kSettingsMicOnRequest,
       std::bind(&RemoteServer::OnSettingsMicOnRequest,
           this, std::placeholders::_1, std::placeholders::_2)},
      {RemoteMessage::MessageType::kSettingsMicOffRequest,
       std::bind(&RemoteServer::OnSettingsMicOffRequest,
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


void RemoteServer::OnCommentsRequest(
  const websocketpp::connection_hdl &connection,
  const boost::property_tree::ptree &tree) {
  const std::string &created_time = tree.get("createdTime", "");

  int request_key = request_cache_.CheckIn(connection);

  StreamingService::Get()->GetComments(
      created_time,
      [this, request_key](const std::string &error) {
    if (error == "not ready") {
      ResponseComments(request_key, "comments not ready", "");
    } else {
      ResponseComments(request_key, "comments error", "");
    }}, [this, request_key](const std::string &comments) {
    ResponseComments(request_key, "", comments);
  });
}


void RemoteServer::OnViewersRequest(
  const websocketpp::connection_hdl &connection,
  const boost::property_tree::ptree &tree) {
  int request_key = request_cache_.CheckIn(connection);

  StreamingService::Get()->GetLiveVideoViewers(
      [this, request_key](const std::string &error) {
    ResponseViewers(request_key, error, "");
  }, [this, request_key](const std::string& viewers) {
    ResponseViewers(request_key, "", viewers);
  });
}


void RemoteServer::OnSettingsWebcamSearchRequest(
    const websocketpp::connection_hdl &connection,
    const boost::property_tree::ptree &tree) {
  int request_key = request_cache_.CheckIn(connection);

  const std::vector<Obs::WebcamDevice> &webcams{
      Obs::Get()->SearchWebcamDevices()};
  RespondSettingsWebcamSearch(request_key, "", webcams);
}


void RemoteServer::OnSettingsWebcamOnRequest(
    const websocketpp::connection_hdl &connection,
    const boost::property_tree::ptree &tree) {
  std::string error{};
  std::string device_id{};
  float width;
  float height;
  float x;
  float y;
  try {
    device_id = tree.get<std::string>("device_id");
    width = tree.get<float>("normal_width");
    height = tree.get<float>("normal_height");
    x = tree.get<float>("normal_x");
    y = tree.get<float>("normal_y");
  } catch (const std::exception &/*e*/) {
    error = "webcam on error";
  }

  if (width > 1.0 || width <= 0.0 ||
      height > 1.0 || height <= 0.0 ||
      x > 1.0 || x < 0.0 ||
      y > 1.0 || y < 0.0) {
    error = "webcam on error";
  }

  int request_key = request_cache_.CheckIn(connection);

  if (error.empty() == false) {
    LogError("OnSettingsWebcamOn: " + error);
    RespondSettingsWebcamOn(request_key, error);
    return;
  }

  const bool &result = Obs::Get()->TurnOnWebcam(device_id, &error);
  if (result == false) {
    if (error != "no device ID") {
      boost::property_tree::ptree args;
      args.add("deviceId", device_id);
      args.add("normalWidth", width);
      args.add("normalHeight", height);
      args.add("normalX", x);
      args.add("normalY", y);

      JsExecutor::Execute(
          browser_app_->GetMainBrowser(),
          "remote.onSettingsWebcamOnRequest",
          args);
    }
    // pretend to success if error is empty.
    RespondSettingsWebcamOn(request_key, error);
    return;
  }

  if (Obs::Get()->UpdateWebcamSize(width, height) == false) {
    error = "webcam size error";
  }

  if (Obs::Get()->UpdateWebcamPosition(x, y) == false) {
    error = "webcam position error";
  }

  boost::property_tree::ptree args;
  args.add("deviceId", device_id);
  args.add("normalWidth", width);
  args.add("normalHeight", height);
  args.add("normalX", x);
  args.add("normalY", y);
  JsExecutor::Execute(
      browser_app_->GetMainBrowser(),
      "remote.onSettingsWebcamOnRequest",
      args);
  RespondSettingsWebcamOn(request_key, error);
}


void RemoteServer::OnSettingsWebcamOffRequest(
    const websocketpp::connection_hdl &connection,
    const boost::property_tree::ptree &tree) {
  int request_key = request_cache_.CheckIn(connection);

  Obs::Get()->TurnOffWebcam();
  JsExecutor::Execute(
      browser_app_->GetMainBrowser(),
      "remote.onSettingsWebcamOffRequest");

  RespondSettingsWebcamOff(request_key, "");
}


void RemoteServer::OnSettingsWebcamSizeRequest(
    const websocketpp::connection_hdl &connection,
    const boost::property_tree::ptree &tree) {
  std::string error{};
  float width;
  float height;
  try {
    width = tree.get<float>("normal_width");
    height = tree.get<float>("normal_height");
  } catch (const std::exception &/*e*/) {
    error = "webcam size error";
  }

  if (width > 1.0 || width <= 0.0 ||
      height > 1.0 || height <= 0.0) {
    error = "webcam size error";
  }

  int request_key = request_cache_.CheckIn(connection);

  if (error.empty() == false) {
    LogError("OnSettingsWebcamSize: " + error);
    RespondSettingsWebcamSize(request_key, error);
    return;
  }

  Obs::Get()->UpdateWebcamSize(width, height);

  boost::property_tree::ptree args;
  args.add("normalWidth", width);
  args.add("normalHeight", height);

  JsExecutor::Execute(
      browser_app_->GetMainBrowser(),
      "remote.onSettingsWebcamSizeRequest",
      args);

  RespondSettingsWebcamSize(request_key, error);
}


void RemoteServer::OnSettingsWebcamPositionRequest(
    const websocketpp::connection_hdl &connection,
    const boost::property_tree::ptree &tree) {
  std::string error{};
  float x;
  float y;
  try {
    x = tree.get<float>("normal_x");
    y = tree.get<float>("normal_y");
  } catch (const std::exception &/*e*/) {
    error = "webcam position error";
  }

  if (x > 1.0 || x < 0.0 ||
      y > 1.0 || y < 0.0) {
    error = "webcam position error";
  }

  int request_key = request_cache_.CheckIn(connection);

  if (error.empty() == false) {
    LogError("OnSettingsWebcamPosition: " + error);
    RespondSettingsWebcamPosition(request_key, error);
    return;
  }

  Obs::Get()->UpdateWebcamPosition(x, y);

  boost::property_tree::ptree args;
  args.add("normalX", x);
  args.add("normalY", y);

  JsExecutor::Execute(
      browser_app_->GetMainBrowser(),
      "remote.onSettingsWebcamPositionRequest",
      args);

  RespondSettingsWebcamPosition(request_key, error);
}


void RemoteServer::OnSettingsChromaKeyOnRequest(
    const websocketpp::connection_hdl &connection,
    const boost::property_tree::ptree &tree) {
  std::string error{};
  uint32_t color{0};
  int similarity{0};
  try {
    color = tree.get<uint32_t>("color");
    similarity = tree.get<int>("similarity");
  } catch (const std::exception &/*e*/) {
    error = "chroma key on error";
  }

  if (similarity > 1000 || similarity < 0) {
    error = "chroma key on error";
  }

  int request_key = request_cache_.CheckIn(connection);

  if (error.empty() == false) {
    LogError("OnSettingsChromaKeyOn: " + error);
    RespondSettingsChromaKeyOn(request_key, error);
    return;
  }

  boost::property_tree::ptree args;
  args.add("color", color);
  args.add("similarity", similarity);

  JsExecutor::Execute(
      browser_app_->GetMainBrowser(),
      "remote.onSettingsChromaKeyOnRequest",
      request_key,
      args);

  RespondSettingsChromaKeyOn(request_key, error);
}


void RemoteServer::OnSettingsChromaKeyOffRequest(
    const websocketpp::connection_hdl &connection,
    const boost::property_tree::ptree &tree) {
  int request_key = request_cache_.CheckIn(connection);

  JsExecutor::Execute(
      browser_app_->GetMainBrowser(),
      "remote.onSettingsChromaKeyOffRequest",
      request_key);

  RespondSettingsChromaKeyOff(request_key, "");
}


void RemoteServer::OnSettingsChromaKeyColorRequest(
    const websocketpp::connection_hdl &connection,
    const boost::property_tree::ptree &tree) {
  std::string error{};
  uint32_t color{0};
  try {
    color = tree.get<uint32_t>("color");
  } catch (const std::exception &/*e*/) {
    error = "chroma key color error";
  }

  int request_key = request_cache_.CheckIn(connection);

  if (error.empty() == false) {
    LogError("OnSettingsChromaKeyColor: " + error);
    RespondSettingsChromaKeyOn(request_key, error);
    return;
  }

  boost::property_tree::ptree args;
  args.add("color", color);

  JsExecutor::Execute(
      browser_app_->GetMainBrowser(),
      "remote.onSettingsChromaKeyColorRequest",
      request_key,
      args);

  RespondSettingsChromaKeyColor(request_key, error);
}


void RemoteServer::OnSettingsChromaKeySimilarityRequest(
    const websocketpp::connection_hdl &connection,
    const boost::property_tree::ptree &tree) {
  std::string error{};
  int similarity{0};
  try {
    similarity = tree.get<int>("similarity");
  } catch (const std::exception &/*e*/) {
    error = "chroma key similarity error";
  }

  if (similarity > 1000 || similarity < 0) {
    error = "chroma key similarity error";
  }

  int request_key = request_cache_.CheckIn(connection);

  if (error.empty() == false) {
    LogError("OnSettingsChromaKeySimilarity: " + error);
    RespondSettingsChromaKeyOn(request_key, error);
    return;
  }

  boost::property_tree::ptree args;
  args.add("similarity", similarity);

  JsExecutor::Execute(
      browser_app_->GetMainBrowser(),
      "remote.onSettingsChromaKeySimilarityRequest",
      request_key,
      args);

  RespondSettingsChromaKeySimilarity(request_key, error);
}


void RemoteServer::OnSettingsMicOnRequest(
    const websocketpp::connection_hdl &connection,
    const boost::property_tree::ptree &tree) {
  int request_key = request_cache_.CheckIn(connection);

  JsExecutor::Execute(
      browser_app_->GetMainBrowser(),
      "remote.onSettingsMicOnRequest",
      request_key);

  RespondSettingsMicOn(request_key, "");
}


void RemoteServer::OnSettingsMicOffRequest(
    const websocketpp::connection_hdl &connection,
    const boost::property_tree::ptree &tree) {
  int request_key = request_cache_.CheckIn(connection);

  JsExecutor::Execute(
      browser_app_->GetMainBrowser(),
      "remote.onSettingsMicOffRequest",
      request_key);

  RespondSettingsMicOff(request_key, "");
}


void RemoteServer::OnNcStreamerExitRequest(
    const websocketpp::connection_hdl &/*connection*/,
    const boost::property_tree::ptree &/*tree*/) {
  HWND wnd = browser_app_->GetMainBrowser()->GetHost()->GetWindowHandle();
  ::PostMessage(wnd, WM_CLOSE, NULL, NULL);
}


bool RemoteServer::RespondStreamingStart(
    int request_key,
    const std::string &error) {
  websocketpp::connection_hdl connection = request_cache_.CheckOut(request_key);
  if (!connection.lock()) {
    LogWarning("RespondStreamingStart: !connection.lock()");
    return false;
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
    return false;
  }

  return true;
}


bool RemoteServer::RespondStreamingStop(
    int request_key,
    const std::string &error) {
  websocketpp::connection_hdl connection = request_cache_.CheckOut(request_key);
  if (!connection.lock()) {
    LogWarning("RespondStreamingStop: !connection.lock()");
    return false;
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
    return false;
  }

  return true;
}


bool RemoteServer::RespondSettingsQualityUpdate(
    int request_key,
    const std::string &error) {
  websocketpp::connection_hdl connection = request_cache_.CheckOut(request_key);
  if (!connection.lock()) {
    LogWarning("RespondSettingsQualityUpdate: !connection.lock()");
    return false;
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
    return false;
  }

  return true;
}


bool RemoteServer::RespondSettingsWebcamSearch(
    int request_key,
    const std::string &error,
    const std::vector<Obs::WebcamDevice> &webcams) {
  websocketpp::connection_hdl connection = request_cache_.CheckOut(request_key);
  if (!connection.lock()) {
    LogWarning("RespondSettingsWebcamSearch: !connection.lock()");
    return false;
  }

  std::stringstream msg;
  {
    std::vector<boost::property_tree::ptree> tree_webcams;
    for (const auto &webcam : webcams) {
      tree_webcams.emplace_back(webcam.ToTree());
    }

    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        RemoteMessage::MessageType::kSettingsWebcamSearchResponse));
    tree.put("error", error);
    tree.add_child("webcamList", JsExecutor::ToPtree(tree_webcams));

    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  server_.send(connection, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    LogError(ec.message());
    return false;
  }

  return true;
}


bool RemoteServer::RespondSettingsWebcamOn(
    int request_key,
    const std::string &error) {
  websocketpp::connection_hdl connection = request_cache_.CheckOut(request_key);
  if (!connection.lock()) {
    LogWarning("RespondSettingsWebcamOn: !connection.lock()");
    return false;
  }

  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        RemoteMessage::MessageType::kSettingsWebcamOnResponse));
    tree.put("error", error);

    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  server_.send(connection, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    LogError(ec.message());
    return false;
  }

  return true;
}


bool RemoteServer::RespondSettingsWebcamOff(
    int request_key,
    const std::string &error) {
  websocketpp::connection_hdl connection = request_cache_.CheckOut(request_key);
  if (!connection.lock()) {
    LogWarning("RespondSettingsWebcamOff: !connection.lock()");
    return false;
  }

  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        RemoteMessage::MessageType::kSettingsWebcamOffResponse));
    tree.put("error", error);

    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  server_.send(connection, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    LogError(ec.message());
    return false;
  }

  return true;
}


bool RemoteServer::RespondSettingsWebcamSize(
    int request_key,
    const std::string &error) {
  websocketpp::connection_hdl connection = request_cache_.CheckOut(request_key);
  if (!connection.lock()) {
    LogWarning("RespondSettingsWebcamSize: !connection.lock()");
    return false;
  }

  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        RemoteMessage::MessageType::kSettingsWebcamSizeResponse));
    tree.put("error", error);

    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  server_.send(connection, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    LogError(ec.message());
    return false;
  }

  return true;
}


bool RemoteServer::RespondSettingsWebcamPosition(
    int request_key,
    const std::string &error) {
  websocketpp::connection_hdl connection = request_cache_.CheckOut(request_key);
  if (!connection.lock()) {
    LogWarning("RespondSettingsWebcamPosition: !connection.lock()");
    return false;
  }

  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        RemoteMessage::MessageType::kSettingsWebcamPositionResponse));
    tree.put("error", error);

    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  server_.send(connection, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    LogError(ec.message());
    return false;
  }

  return true;
}


bool RemoteServer::RespondSettingsChromaKeyOn(
    int request_key,
    const std::string &error) {
  websocketpp::connection_hdl connection = request_cache_.CheckOut(request_key);
  if (!connection.lock()) {
    LogWarning("RespondSettingsChromaKeyOn: !connection.lock()");
    return false;
  }

  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        RemoteMessage::MessageType::kSettingsChromaKeyOnResponse));
    tree.put("error", error);

    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  server_.send(connection, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    LogError(ec.message());
    return false;
  }

  return true;
}


bool RemoteServer::RespondSettingsChromaKeyOff(
    int request_key,
    const std::string &error) {
  websocketpp::connection_hdl connection = request_cache_.CheckOut(request_key);
  if (!connection.lock()) {
    LogWarning("RespondSettingsChromaKeyOff: !connection.lock()");
    return false;
  }

  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        RemoteMessage::MessageType::kSettingsChromaKeyOffResponse));
    tree.put("error", error);

    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  server_.send(connection, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    LogError(ec.message());
    return false;
  }

  return true;
}


bool RemoteServer::RespondSettingsChromaKeyColor(
    int request_key,
    const std::string &error) {
  websocketpp::connection_hdl connection = request_cache_.CheckOut(request_key);
  if (!connection.lock()) {
    LogWarning("RespondSettingsChromaKeyColor: !connection.lock()");
    return false;
  }

  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        RemoteMessage::MessageType::kSettingsChromaKeyColorResponse));
    tree.put("error", error);

    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  server_.send(connection, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    LogError(ec.message());
    return false;
  }

  return true;
}


bool RemoteServer::RespondSettingsChromaKeySimilarity(
    int request_key,
    const std::string &error) {
  websocketpp::connection_hdl connection = request_cache_.CheckOut(request_key);
  if (!connection.lock()) {
    LogWarning("RespondSettingsChromaKeySimilarity: !connection.lock()");
    return false;
  }

  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        RemoteMessage::MessageType::kSettingsChromaKeySimilarityResponse));
    tree.put("error", error);

    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  server_.send(connection, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    LogError(ec.message());
    return false;
  }

  return true;
}


bool RemoteServer::RespondSettingsMicOn(
    int request_key,
    const std::string &error) {
  websocketpp::connection_hdl connection = request_cache_.CheckOut(request_key);
  if (!connection.lock()) {
    LogWarning("RespondSettingsMicOn: !connection.lock()");
    return false;
  }

  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        RemoteMessage::MessageType::kSettingsMicOnResponse));
    tree.put("error", error);

    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  server_.send(connection, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    LogError(ec.message());
    return false;
  }

  return true;
}


bool RemoteServer::RespondSettingsMicOff(
    int request_key,
    const std::string &error) {
  websocketpp::connection_hdl connection = request_cache_.CheckOut(request_key);
  if (!connection.lock()) {
    LogWarning("RespondSettingsMicOff: !connection.lock()");
    return false;
  }

  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        RemoteMessage::MessageType::kSettingsMicOffResponse));
    tree.put("error", error);

    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  server_.send(connection, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    LogError(ec.message());
    return false;
  }

  return true;
}


void RemoteServer::BroadcastStreamingStart(
    const std::string &source,
    const std::string &user_page,
    const std::string &privacy,
    const std::string &description,
    const std::string &mic,
    const std::string &service_provider,
    const std::string &stream_url,
    const std::string &post_url) {
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
    tree.put("postUrl", post_url);
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
