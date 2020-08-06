/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include <memory>

#include "boost/filesystem.hpp"
#include "windows.h"  // NOLINT

#include "ncstreamer_cef/src/browser_app.h"
#include "ncstreamer_cef/src/command_line.h"
#include "ncstreamer_cef/src/lib/named_mutex.h"
#include "ncstreamer_cef/src/lib/window_frame_remover.h"
#include "ncstreamer_cef/src/lib/windows_types.h"
#include "ncstreamer_cef/src/local_storage.h"
#include "ncstreamer_cef/src/manifest.h"
#include "ncstreamer_cef/src/obs.h"
#include "ncstreamer_cef/src/render_app.h"


namespace {
int ExecuteProcess(HINSTANCE instance) {
  CefRefPtr<ncstreamer::RenderApp> render_app{new ncstreamer::RenderApp{}};
  return ::CefExecuteProcess(
      CefMainArgs{instance}, render_app, nullptr);
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
  const int &exit_code = ExecuteProcess(instance);
  if (exit_code >= 0) {
    return -1;
  }

  ncstreamer::NamedMutex named_mutex{ncstreamer::kMutexName};
  if (true == named_mutex.IsMutexRegistered()) {
    HWND prev_instance = ::FindWindow(NULL, ncstreamer::kAppName);
    if (prev_instance != NULL) {
      ::ShowWindow(prev_instance, SW_RESTORE);
      ::SetForegroundWindow(prev_instance);
    }
    return -1;
  }

  if (false == named_mutex.RegisterMutex()) {
    return -1;
  }

  CefRefPtr<ncstreamer::BrowserApp> browser_app{new ncstreamer::BrowserApp{
      instance,
      cmd_line}};

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

  ncstreamer::WindowFrameRemover::ShutDown();
  ncstreamer::LocalStorage::ShutDown();
  ::CefShutdown();

  return 0;
}
