/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include <cassert>
#include <memory>

#include "windows.h"  // NOLINT

#include "ncstreamer_cef/src/browser_app.h"
#include "ncstreamer_cef/src/command_line.h"
#include "ncstreamer_cef/src/lib/window_frame_remover.h"
#include "ncstreamer_cef/src/obs.h"
#include "ncstreamer_cef/src/remote_server.h"
#include "ncstreamer_cef/src/render_app.h"
#include "ncstreamer_cef/src/streaming_service.h"


int APIENTRY wWinMain(HINSTANCE instance,
                      HINSTANCE /*prev_instance*/,
                      LPTSTR /*cmd_line_str*/,
                      int /*cmd_show*/) {
  ::CefEnableHighDPISupport();
  CefMainArgs main_args{instance};
  ncstreamer::CommandLine cmd_line{::GetCommandLine()};

  if (cmd_line.is_renderer()) {
    CefRefPtr<ncstreamer::RenderApp> render_app{new ncstreamer::RenderApp{}};
    int exit_code = ::CefExecuteProcess(main_args, render_app, nullptr);
    assert(exit_code >= 0);
    return exit_code;
  }

  CefRefPtr<ncstreamer::BrowserApp> browser_app{new ncstreamer::BrowserApp{
          instance,
          cmd_line.shows_sources_all(),
          cmd_line.sources(),
          cmd_line.locale(),
          cmd_line.ui_uri()}};

  CefSettings settings;
  settings.no_sandbox = true;

  ::CefInitialize(main_args, settings, browser_app, nullptr);
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
  ::CefShutdown();

  return 0;
}
