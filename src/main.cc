/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include <memory>

#include "windows.h"  // NOLINT

#include "src/app.h"
#include "src/obs.h"


int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE /*hPrevInstance*/,
                      LPTSTR /*lpCmdLine*/,
                      int /*nCmdShow*/) {
  ::CefEnableHighDPISupport();
  CefMainArgs main_args{hInstance};

  CefRefPtr<ncstreamer::App> app{new ncstreamer::App{hInstance}};

  int exit_code = ::CefExecuteProcess(main_args, nullptr, nullptr);
  if (exit_code >= 0) {
    return exit_code;
  }

  CefSettings settings;
  settings.no_sandbox = true;

  ::CefInitialize(main_args, settings, app, nullptr);
  ncstreamer::Obs::SetUp();

  ::CefRunMessageLoop();

  ncstreamer::Obs::ShutDown();
  ::CefShutdown();

  return 0;
}
