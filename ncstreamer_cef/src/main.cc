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

  HWND prev_instance = ::FindWindow(NULL, ncstreamer::kAppName);
  if (prev_instance != NULL) {
    ::ShowWindow(prev_instance, SW_RESTORE);
    ::SetForegroundWindow(prev_instance);
    return -1;
  }

  CefRefPtr<ncstreamer::BrowserApp> browser_app{new ncstreamer::BrowserApp{
      instance,
      cmd_line.hides_settings(),
      cmd_line.video_quality(),
      cmd_line.shows_sources_all(),
      cmd_line.sources(),
      cmd_line.locale(),
      cmd_line.ui_uri(),
      cmd_line.default_position(),
      cmd_line.streaming_service_tag_ids(),
      cmd_line.designated_user(),
      cmd_line.device_settings()}};

  ncstreamer::RemoteServer::SetUp(browser_app);
  bool started = ncstreamer::RemoteServer::Get()->Start(cmd_line.remote_port());
  if (started == false) {
    return -1;
  }

  auto app_data_path = CreateUserLocalAppDirectory();
  boost::filesystem::path storage_path{};
  if (cmd_line.in_memory_local_storage() == false) {
    storage_path = app_data_path / L"local_storage.json";
  }
  ncstreamer::LocalStorage::SetUp(storage_path.c_str());

  CefSettings settings;
  settings.no_sandbox = true;
  if (cmd_line.in_memory_local_storage() == false) {
    CefString(&settings.cache_path) = (app_data_path / L"cef_cache").c_str();
  }

  ::CefInitialize(CefMainArgs{instance}, settings, browser_app, nullptr);

  ncstreamer::WindowFrameRemover::SetUp();

  ::CefRunMessageLoop();

  ncstreamer::RemoteServer::ShutDown();
  ncstreamer::WindowFrameRemover::ShutDown();
  ncstreamer::LocalStorage::ShutDown();
  ::CefShutdown();

  return 0;
}
