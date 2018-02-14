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
    bool hides_settings,
    const std::wstring &video_quality,
    bool shows_sources_all,
    const std::vector<std::string> &sources,
    const std::wstring &locale,
    const std::wstring &ui_uri,
    const Position<int> &default_position,
    const StreamingServiceTagMap &tag_ids,
    const std::wstring &designated_user,
    const boost::property_tree::ptree &device_settings,
    const uint16_t &remote_port)
    : instance_{instance},
      hides_settings_{hides_settings},
      video_quality_{video_quality},
      shows_sources_all_{shows_sources_all},
      sources_{sources},
      locale_{locale},
      ui_uri_{ui_uri},
      defalut_position_{default_position},
      tag_ids_{tag_ids},
      designated_user_{designated_user},
      client_{},
      device_settings_{device_settings},
      remote_port_{remote_port} {
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
      remote_port_};

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
