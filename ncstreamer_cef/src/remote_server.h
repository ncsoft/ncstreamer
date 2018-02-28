/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_REMOTE_SERVER_H_
#define NCSTREAMER_CEF_SRC_REMOTE_SERVER_H_


#include <fstream>
#include <mutex>  // NOLINT
#include <string>
#include <thread>  // NOLINT
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "boost/asio/io_service.hpp"
#include "boost/property_tree/ptree.hpp"
#include "include/cef_app.h"
#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"

#include "ncstreamer_cef/src/obs.h"


namespace ncstreamer {
class RemoteServer {
 public:
  static void SetUp(
      const CefRefPtr<CefBrowser> browser_app);

  static void ShutDown();
  static RemoteServer *Get();

  bool Start(uint16_t port);

  void RespondStreamingStatus(
      int request_key,
      const std::string &status,
      const std::string &source_title,
      const std::string &user_name,
      const std::string &quality);

  void ResponseComments(
      int request_key,
      const std::string &error,
      const std::string &comments);

  void ResponseViewers(
      int request_key,
      const std::string &error,
      const std::string &viewers);

  void NotifyStreamingStart(
      int request_key,
      const std::string &error,
      const std::string &source,
      const std::string &user_page,
      const std::string &privacy,
      const std::string &description,
      const std::string &mic,
      const std::string &service_provider,
      const std::string &stream_url,
      const std::string &post_url);

  void NotifyStreamingStop(
      int request_key,
      const std::string &error,
      const std::string &source);

  void NotifySettingsQualityUpdate(
      int request_key,
      const std::string &error);

 private:
  class ConnectionHasher {
   public:
    std::size_t operator()(
        const websocketpp::connection_hdl &connection) const;
  };

  class ConnectionKeyeq {
   public:
    bool operator()(
        const websocketpp::connection_hdl &left,
        const websocketpp::connection_hdl &right) const;
  };

  class RequestCache {
   public:
    RequestCache();
    virtual ~RequestCache();

    int CheckIn(websocketpp::connection_hdl connection);
    websocketpp::connection_hdl CheckOut(int key);

   private:
    mutable std::mutex mutex_;
    std::unordered_map<int /*key*/, websocketpp::connection_hdl> cache_;
    int last_key_;
  };

  RemoteServer(
      const CefRefPtr<CefBrowser> browser_app);

  virtual ~RemoteServer();

  void OnFail(websocketpp::connection_hdl connection);
  void OnOpen(websocketpp::connection_hdl connection);
  void OnClose(websocketpp::connection_hdl connection);
  void OnMessage(
      websocketpp::connection_hdl connection,
      websocketpp::connection<websocketpp::config::asio>::message_ptr msg);

  void OnStreamingStatusRequest(
      const websocketpp::connection_hdl &connection,
      const boost::property_tree::ptree &tree);

  void OnStreamingStartRequest(
      const websocketpp::connection_hdl &connection,
      const boost::property_tree::ptree &tree);

  void OnStreamingStopRequest(
      const websocketpp::connection_hdl &connection,
      const boost::property_tree::ptree &tree);

  void OnSettingsQualityUpdateRequest(
      const websocketpp::connection_hdl &connection,
      const boost::property_tree::ptree &tree);

  void OnCommentsRequest(
      const websocketpp::connection_hdl &connection,
      const boost::property_tree::ptree &tree);

  void OnViewersRequest(
      const websocketpp::connection_hdl &connection,
      const boost::property_tree::ptree &tree);

  void OnSettingsWebcamSearchRequest(
      const websocketpp::connection_hdl &connection,
      const boost::property_tree::ptree &tree);

  void OnSettingsWebcamOnRequest(
      const websocketpp::connection_hdl &connection,
      const boost::property_tree::ptree &tree);

  void OnSettingsWebcamOffRequest(
      const websocketpp::connection_hdl &connection,
      const boost::property_tree::ptree &tree);

  void OnSettingsWebcamSizeRequest(
      const websocketpp::connection_hdl &connection,
      const boost::property_tree::ptree &tree);

  void OnSettingsWebcamPositionRequest(
      const websocketpp::connection_hdl &connection,
      const boost::property_tree::ptree &tree);

  void OnSettingsChromaKeyOnRequest(
      const websocketpp::connection_hdl &connection,
      const boost::property_tree::ptree &tree);

  void OnSettingsChromaKeyOffRequest(
      const websocketpp::connection_hdl &connection,
      const boost::property_tree::ptree &tree);

  void OnSettingsChromaKeyColorRequest(
      const websocketpp::connection_hdl &connection,
      const boost::property_tree::ptree &tree);

  void OnSettingsChromaKeySimilarityRequest(
      const websocketpp::connection_hdl &connection,
      const boost::property_tree::ptree &tree);

  void OnSettingsMicSearchRequest(
      const websocketpp::connection_hdl &connection,
      const boost::property_tree::ptree &tree);

  void OnSettingsMicOnRequest(
      const websocketpp::connection_hdl &connection,
      const boost::property_tree::ptree &tree);

  void OnSettingsMicOffRequest(
      const websocketpp::connection_hdl &connection,
      const boost::property_tree::ptree &tree);

  void OnNcStreamerExitRequest(
      const websocketpp::connection_hdl &connection,
      const boost::property_tree::ptree &tree);

  bool RespondStreamingStart(
      int request_key,
      const std::string &error);

  bool RespondStreamingStop(
      int request_key,
      const std::string &error);

  bool RespondSettingsQualityUpdate(
      int request_key,
      const std::string &error);

  bool RespondSettingsWebcamSearch(
      int request_key,
      const std::string &error,
      const std::vector<std::string> &webcams);

  bool RespondSettingsWebcamOn(
      int request_key,
      const std::string &error);

  bool RespondSettingsWebcamOff(
      int request_key,
      const std::string &error);

  bool RespondSettingsWebcamSize(
      int request_key,
      const std::string &error);

  bool RespondSettingsWebcamPosition(
      int request_key,
      const std::string &error);

  bool RespondSettingsChromaKeyOn(
      int request_key,
      const std::string &error);

  bool RespondSettingsChromaKeyOff(
      int request_key,
      const std::string &error);

  bool RespondSettingsChromaKeyColor(
      int request_key,
      const std::string &error);

  bool RespondSettingsChromaKeySimilarity(
      int request_key,
      const std::string &error);

  bool RespondSettingsMicSearch(
      int request_key,
      const std::string &error,
      const std::unordered_map<std::string, std::string> &mic_devices);

  bool RespondSettingsMicOn(
      int request_key,
      const std::string &error);

  bool RespondSettingsMicOff(
      int request_key,
      const std::string &error);

  void BroadcastStreamingStart(
      const std::string &source,
      const std::string &user_page,
      const std::string &privacy,
      const std::string &description,
      const std::string &mic,
      const std::string &service_provider,
      const std::string &stream_url,
      const std::string &post_url);

  void BroadcastStreamingStop(
      const std::string &source);

  void Broadcast(const std::string &msg);

  void LogError(const std::string &err_msg);
  void LogWarning(const std::string &warn_msg);
  void LogInfo(const std::string &info_msg);

  static RemoteServer *static_instance;

  const CefRefPtr<CefBrowser> browser_;

  boost::asio::io_service io_service_;
  boost::asio::io_service::work io_service_work_;
  websocketpp::server<websocketpp::config::asio> server_;
  std::vector<std::thread> server_threads_;
  std::ofstream server_log_;

  std::unordered_set<websocketpp::connection_hdl,
      ConnectionHasher, ConnectionKeyeq> connections_;

  RequestCache request_cache_;
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_REMOTE_SERVER_H_
