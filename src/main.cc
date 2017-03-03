/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include <memory>

#include "windows.h"  // NOLINT

#include "src/app.h"
#include "src/obs.h"


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE /*hPrevInstance*/,
                     LPTSTR /*lpCmdLine*/,
                     int /*nCmdShow*/) {
  ::CefEnableHighDPISupport();
  CefMainArgs main_args{hInstance};

  int exit_code = ::CefExecuteProcess(main_args, nullptr, nullptr);
  if (exit_code >= 0) {
    return exit_code;
  }

  CefSettings settings;
  settings.no_sandbox = true;
  std::wstring temp_path = []() -> std::wstring {
    uint32 len = ::GetTempPathW(0, NULL);
    std::unique_ptr<wchar_t[]> buf{new wchar_t[len]};
    ::GetTempPathW(len, buf.get());
    return buf.get();
  }();
  temp_path += L"cef_cache";
  CefString(&settings.cache_path) = temp_path.c_str();

  CefRefPtr<ncstreamer::App> app{new ncstreamer::App{hInstance}};

  ::CefInitialize(main_args, settings, app, nullptr);
  ncstreamer::Obs::SetUp();

  ::CefRunMessageLoop();

  ncstreamer::Obs::ShutDown();
  ::CefShutdown();

  return 0;
}
