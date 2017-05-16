/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_CLIENT_CLIENT_REQUEST_HANDLER_H_
#define NCSTREAMER_CEF_SRC_CLIENT_CLIENT_REQUEST_HANDLER_H_

#include <functional>
#include <string>
#include <unordered_map>

#include "include/cef_request_handler.h"


namespace ncstreamer {
class ClientRequestHandler : public CefRequestHandler {
 public:
  explicit ClientRequestHandler(
      const std::wstring &locale);

  virtual ~ClientRequestHandler();

  bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                      CefRefPtr<CefFrame> frame,
                      CefRefPtr<CefRequest> request,
                      bool is_redirect) override;

 private:
  using CommandArgumentMap = std::unordered_map<std::string, std::string>;

  static CommandArgumentMap ParseQuery(const std::string &query);
  static std::string DecodeUri(const std::string &enc_string);

  void OnCommand(const std::string &cmd,
                 const CommandArgumentMap &args,
                 CefRefPtr<CefBrowser> browser);

  // command handlers
  using CommandHandler = std::function<void (
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser)>;
  void OnCommandWindowClose(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandWindowMinimize(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandExternalBrowserPopUp(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandServiceProviderLogIn(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandServiceProviderLogOut(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandStreamingStart(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandStreamingStop(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandSettingsMicOn(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandSettingsMicOff(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandSettingsMicVolumeUpdate(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandSettingsVideoQualityUpdate(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandStorageUserPageUpdate(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandStoragePrivacyUpdate(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandRemoteStatus(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandRemoteStart(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandRemoteStop(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandRemoteQualityUpdate(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);

  std::wstring locale_;

  IMPLEMENT_REFCOUNTING(ClientRequestHandler);
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_CLIENT_CLIENT_REQUEST_HANDLER_H_
