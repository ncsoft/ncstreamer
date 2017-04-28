/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include <memory>

#include "windows.h"  // NOLINT

#include "ncstreamer_cef/src/browser_app.h"
#include "ncstreamer_cef/src/command_line.h"
#include "ncstreamer_cef/src/lib/window_frame_remover.h"
#include "ncstreamer_cef/src/obs.h"
#include "ncstreamer_cef/src/render_app.h"
#include "ncstreamer_cef/src/streaming_service.h"


int APIENTRY wWinMain(HINSTANCE instance,
                      HINSTANCE /*prev_instance*/,
                      LPTSTR /*cmd_line_str*/,
                      int /*cmd_show*/) {
  ::CefEnableHighDPISupport();
  CefMainArgs main_args{instance};
  ncstreamer::CommandLine cmd_line{::GetCommandLine()};

  auto app = cmd_line.is_renderer() ?
      CefRefPtr<CefApp>{new ncstreamer::RenderApp{}} :
      CefRefPtr<CefApp>{new ncstreamer::BrowserApp{
          instance,
          cmd_line.shows_sources_all(),
          cmd_line.sources(),
          cmd_line.locale(),
          cmd_line.ui_uri()}};

  int exit_code = ::CefExecuteProcess(main_args, app, nullptr);
  if (exit_code >= 0) {
    return exit_code;
  }

  CefSettings settings;
  settings.no_sandbox = true;

  ::CefInitialize(main_args, settings, app, nullptr);
  ncstreamer::WindowFrameRemover::SetUp();
  ncstreamer::Obs::SetUp();
  ncstreamer::StreamingService::SetUp();

  ::CefRunMessageLoop();

  ncstreamer::StreamingService::ShutDown();
  ncstreamer::Obs::ShutDown();
  ncstreamer::WindowFrameRemover::ShutDown();
  ::CefShutdown();

  return 0;
}
