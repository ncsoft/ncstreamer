/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_REMOTE_SERVER_H_
#define NCSTREAMER_CEF_SRC_REMOTE_SERVER_H_


#include <fstream>
#include <thread>  // NOLINT
#include <vector>

#include "boost/asio/io_service.hpp"
#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"

#include "ncstreamer_cef/src/browser_app.h"


namespace ncstreamer {
namespace ws = websocketpp;


class RemoteServer {
 public:
  static void SetUp(
      const BrowserApp *browser_app,
      uint16_t port);

  static void ShutDown();
  static RemoteServer *Get();

 private:
  using Asio = ws::config::asio;

  RemoteServer(
      const BrowserApp *browser_app,
      uint16_t port);

  virtual ~RemoteServer();

  void OnFail(ws::connection_hdl connection);
  void OnOpen(ws::connection_hdl connection);
  void OnClose(ws::connection_hdl connection);
  void OnMessage(ws::connection_hdl connection,
                 ws::connection<Asio>::message_ptr msg);

  static RemoteServer *static_instance;

  const BrowserApp *const browser_app_;

  boost::asio::io_service io_service_;
  boost::asio::io_service::work io_service_work_;
  ws::server<Asio> server_;
  std::vector<std::thread> server_threads_;
  std::ofstream server_log_;
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_REMOTE_SERVER_H_
