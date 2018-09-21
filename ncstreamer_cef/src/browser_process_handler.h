/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_BROWSER_PROCESS_HANDLER_H_
#define NCSTREAMER_CEF_SRC_BROWSER_PROCESS_HANDLER_H_


#include <string>
#include <vector>

#include "boost/property_tree/ptree.hpp"
#include "include/cef_browser_process_handler.h"
#include "windows.h"  // NOLINT

#include "ncstreamer_cef/src/client.h"
#include "ncstreamer_cef/src/command_line.h"
#include "ncstreamer_cef/src/lib/position.h"
#include "ncstreamer_cef/src/lib/rectangle.h"
#include "ncstreamer_cef/src/streaming_service/streaming_service_types.h"


namespace ncstreamer {
class BrowserProcessHandler : public CefBrowserProcessHandler {
 public:
  BrowserProcessHandler(
      HINSTANCE instance,
      const CommandLine &cmd_line);
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
  const boost::property_tree::ptree device_settings_;
  const uint16_t remote_port_;
  const std::wstring location_;
  const std::wstring uid_hash_;

  CefRefPtr<Client> client_;

  IMPLEMENT_REFCOUNTING(BrowserProcessHandler);
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_BROWSER_PROCESS_HANDLER_H_
