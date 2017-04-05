/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/browser_process_handler.h"

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/wrapper/cef_helpers.h"

#include "src/client.h"
#include "src/lib/display.h"
#include "src/lib/window_frame_remover.h"
#include "src/manifest.h"


namespace ncstreamer {
BrowserProcessHandler::BrowserProcessHandler(
    HINSTANCE instance,
    bool needs_to_find_sources,
    const std::vector<std::string> &sources)
    : instance_{instance},
      needs_to_find_sources_{needs_to_find_sources},
      sources_{sources} {
}


BrowserProcessHandler::~BrowserProcessHandler() {
}


void BrowserProcessHandler::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  CefWindowInfo window_info;
  window_info.style = WindowFrameRemover::kWindowStyleBeforeInitialization;
  window_info.x = CW_USEDEFAULT;
  window_info.y = CW_USEDEFAULT;
  window_info.width = Display::Scale(kWindowMinimumSize.width());
  window_info.height = Display::Scale(kWindowMinimumSize.height());

  CefRefPtr<Client> client{new Client{
      instance_,
      needs_to_find_sources_,
      sources_}};

  std::wstring uri{
      CefCommandLine::GetGlobalCommandLine()->GetSwitchValue(L"ui-uri")};
  if (uri.empty() == true) {
    uri = kDefaultUiUri;
  }

  CefBrowserSettings browser_settings;

  CefBrowserHost::CreateBrowser(
      window_info, client, uri, browser_settings, NULL);
}
}  // namespace ncstreamer
