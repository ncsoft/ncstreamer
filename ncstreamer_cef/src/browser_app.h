/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_BROWSER_APP_H_
#define NCSTREAMER_CEF_SRC_BROWSER_APP_H_


#include <string>
#include <vector>

#include "include/cef_app.h"
#include "windows.h"  // NOLINT

#include "ncstreamer_cef/src/browser_process_handler.h"
#include "ncstreamer_cef/src/lib/position.h"
#include "ncstreamer_cef/src/streaming_service/streaming_service_types.h"


namespace ncstreamer {
class BrowserApp : public CefApp {
 public:
  BrowserApp(
      HINSTANCE instance,
      bool hides_settings,
      const std::wstring &video_quality,
      bool shows_sources_all,
      const std::vector<std::string> &sources,
      const std::wstring &locale,
      const std::wstring &ui_uri,
      const Position<int> &default_position,
      const StreamingServiceTagMap &tag_ids,
      const std::wstring &designated_user);
  virtual ~BrowserApp();

  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override;

  const CefRefPtr<CefBrowser> &GetMainBrowser() const;

 private:
  CefRefPtr<BrowserProcessHandler> browser_process_handler_;

  IMPLEMENT_REFCOUNTING(BrowserApp);
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_BROWSER_APP_H_
