/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_BROWSER_PROCESS_HANDLER_H_
#define NCSTREAMER_CEF_SRC_BROWSER_PROCESS_HANDLER_H_


#include <string>
#include <vector>

#include "include/cef_browser_process_handler.h"
#include "windows.h"  // NOLINT

#include "ncstreamer_cef/src/client.h"
#include "ncstreamer_cef/src/lib/position.h"
#include "ncstreamer_cef/src/lib/rectangle.h"
#include "ncstreamer_cef/src/streaming_service/streaming_service_types.h"


namespace ncstreamer {
class BrowserProcessHandler : public CefBrowserProcessHandler {
 public:
  BrowserProcessHandler(
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
  virtual ~BrowserProcessHandler();

  // override CefBrowserProcessHandler
  void OnContextInitialized() override;

  const CefRefPtr<CefBrowser> &GetMainBrowser() const;

 private:
  Rectangle LoadWindowRectangle();

  const HINSTANCE instance_;
  const bool hides_settings_;
  const std::wstring video_quality_;
  const bool shows_sources_all_;
  const std::vector<std::string> sources_;
  const std::wstring locale_;
  const std::wstring ui_uri_;
  const Position<int> defalut_position_;
  const StreamingServiceTagMap tag_ids_;
  const std::wstring designated_user_;

  CefRefPtr<Client> client_;

  IMPLEMENT_REFCOUNTING(BrowserProcessHandler);
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_BROWSER_PROCESS_HANDLER_H_
