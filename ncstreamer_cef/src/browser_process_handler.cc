/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/browser_process_handler.h"

#include "boost/algorithm/string/replace.hpp"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/wrapper/cef_helpers.h"

#include "ncstreamer_cef/src/lib/display.h"
#include "ncstreamer_cef/src/lib/monitor_info.h"
#include "ncstreamer_cef/src/lib/position.h"
#include "ncstreamer_cef/src/lib/window_frame_remover.h"
#include "ncstreamer_cef/src/local_storage.h"
#include "ncstreamer_cef/src/manifest.h"


namespace ncstreamer {
BrowserProcessHandler::BrowserProcessHandler(
    HINSTANCE instance,
    const CommandLine &cmd_line)
    : instance_{instance},
      hides_settings_{cmd_line.hides_settings()},
      video_quality_{cmd_line.video_quality()},
      shows_sources_all_{cmd_line.shows_sources_all()},
      sources_{cmd_line.sources()},
      locale_{cmd_line.locale()},
      ui_uri_{cmd_line.ui_uri()},
      defalut_position_{cmd_line.default_position()},
      tag_ids_{cmd_line.streaming_service_tag_ids()},
      designated_user_{cmd_line.designated_user()},
      device_settings_{cmd_line.device_settings()},
      remote_port_{cmd_line.remote_port()},
      location_{cmd_line.location()},
      uid_hash_{cmd_line.uid_hash()},
      client_{} {
}


BrowserProcessHandler::~BrowserProcessHandler() {
}


void BrowserProcessHandler::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  const Rectangle &rect = LoadWindowRectangle();

  CefWindowInfo window_info;
  window_info.style = WindowFrameRemover::kWindowStyleBeforeInitialization;
  window_info.x = rect.x();
  window_info.y = rect.y();
  window_info.width = rect.width();
  window_info.height = rect.height();

  client_ = new Client{
      instance_,
      hides_settings_,
      video_quality_,
      shows_sources_all_,
      sources_,
      locale_,
      tag_ids_,
      designated_user_,
      device_settings_,
      remote_port_,
      location_,
      uid_hash_};

  std::wstring uri{ui_uri_};
  if (uri.empty() == true) {
    uri = boost::replace_all_copy(
        kDefaultUiUri, kDefaultUiUriLocaleTemplate, locale_);
  }

  CefBrowserSettings browser_settings;
  CefBrowserHost::CreateBrowser(
      window_info, client_, uri, browser_settings, NULL);
}


Rectangle BrowserProcessHandler::LoadWindowRectangle() {
  const boost::optional<Position<int>> &position =
      LocalStorage::Get()->GetWindowPosition();
  Dimension<int> window_size{Display::Scale(kWindowMinimumSize)};
  if (!position ||
      !MonitorInfo::RectInMonitor({position->x(),
                                   position->y(),
                                   window_size.width(),
                                   window_size.height()})) {
    return {defalut_position_.x(),
            defalut_position_.y(),
            window_size.width(),
            window_size.height()};
  }
  return {position->x(),
          position->y(),
          window_size.width(),
          window_size.height()};
}


const CefRefPtr<CefBrowser> &BrowserProcessHandler::GetMainBrowser() const {
  static const CefRefPtr<CefBrowser> kNullBrowser{};
  return client_ ? client_->GetMainBrowser() : kNullBrowser;
}
}  // namespace ncstreamer
