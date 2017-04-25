/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/browser_process_handler.h"

#include "boost/algorithm/string/replace.hpp"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/wrapper/cef_helpers.h"

#include "ncstreamer_cef/src/client.h"
#include "ncstreamer_cef/src/lib/display.h"
#include "ncstreamer_cef/src/lib/window_frame_remover.h"
#include "ncstreamer_cef/src/manifest.h"


namespace ncstreamer {
BrowserProcessHandler::BrowserProcessHandler(
    HINSTANCE instance,
    bool shows_sources_all,
    const std::vector<std::string> &sources,
    const std::wstring &locale,
    const std::wstring &ui_uri)
    : instance_{instance},
      shows_sources_all_{shows_sources_all},
      sources_{sources},
      locale_{locale},
      ui_uri_{ui_uri} {
}


BrowserProcessHandler::~BrowserProcessHandler() {
}


void BrowserProcessHandler::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  Dimension<int> window_size{Display::Scale(kWindowMinimumSize)};

  CefWindowInfo window_info;
  window_info.style = WindowFrameRemover::kWindowStyleBeforeInitialization;
  window_info.x = CW_USEDEFAULT;
  window_info.y = CW_USEDEFAULT;
  window_info.width = window_size.width();
  window_info.height = window_size.height();

  CefRefPtr<Client> client{new Client{
      instance_,
      shows_sources_all_,
      sources_,
      locale_}};

  std::wstring uri{ui_uri_};
  if (uri.empty() == true) {
    uri = boost::replace_all_copy(
        kDefaultUiUri, kDefaultUiUriLocaleTemplate, locale_);
  }

  CefBrowserSettings browser_settings;

  CefBrowserHost::CreateBrowser(
      window_info, client, uri, browser_settings, NULL);
}
}  // namespace ncstreamer
