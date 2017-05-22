/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include <cassert>
#include <memory>

#include "boost/filesystem.hpp"
#include "windows.h"  // NOLINT

#include "ncstreamer_cef/src/browser_app.h"
#include "ncstreamer_cef/src/command_line.h"
#include "ncstreamer_cef/src/lib/window_frame_remover.h"
#include "ncstreamer_cef/src/lib/windows_types.h"
#include "ncstreamer_cef/src/local_storage.h"
#include "ncstreamer_cef/src/manifest.h"
#include "ncstreamer_cef/src/obs.h"
#include "ncstreamer_cef/src/remote_server.h"
#include "ncstreamer_cef/src/render_app.h"
#include "ncstreamer_cef/src/streaming_service.h"


namespace {
int ExecuteRenderProcess(HINSTANCE instance) {
  CefRefPtr<ncstreamer::RenderApp> render_app{new ncstreamer::RenderApp{}};
  int exit_code = ::CefExecuteProcess(
      CefMainArgs{instance}, render_app, nullptr);
  assert(exit_code >= 0);
  return exit_code;
}


boost::filesystem::path CreateUserLocalAppDirectory() {
  boost::filesystem::path user_data_path{
      ncstreamer::Windows::GetUserLocalAppDataPath()};
  boost::filesystem::path app_data_path{
      user_data_path / ncstreamer::kAppName};

  boost::filesystem::create_directories(app_data_path);
  return app_data_path;
}
}  // unnamed namespace


int APIENTRY wWinMain(HINSTANCE instance,
                      HINSTANCE /*prev_instance*/,
                      LPTSTR /*cmd_line_str*/,
                      int /*cmd_show*/) {
  ::CefEnableHighDPISupport();
  ncstreamer::CommandLine cmd_line{::GetCommandLine()};

  if (cmd_line.is_renderer()) {
    return ExecuteRenderProcess(instance);
  }

  auto app_data_path = CreateUserLocalAppDirectory();

  CefRefPtr<ncstreamer::BrowserApp> browser_app{new ncstreamer::BrowserApp{
      instance,
      cmd_line.shows_sources_all(),
      cmd_line.sources(),
      cmd_line.locale(),
      cmd_line.ui_uri()}};

  CefSettings settings;
  settings.no_sandbox = true;

  ::CefInitialize(CefMainArgs{instance}, settings, browser_app, nullptr);

  auto storage_path = app_data_path / L"local_storage.json";

  ncstreamer::LocalStorage::SetUp(storage_path.c_str());
  ncstreamer::WindowFrameRemover::SetUp();
  ncstreamer::Obs::SetUp();
  ncstreamer::StreamingService::SetUp();
  ncstreamer::RemoteServer::SetUp(
      browser_app,
      cmd_line.remote_port());

  ::CefRunMessageLoop();

  ncstreamer::RemoteServer::ShutDown();
  ncstreamer::StreamingService::ShutDown();
  ncstreamer::Obs::ShutDown();
  ncstreamer::WindowFrameRemover::ShutDown();
  ncstreamer::LocalStorage::ShutDown();
  ::CefShutdown();

  return 0;
}
