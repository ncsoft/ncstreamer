/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_CLIENT_H_
#define NCSTREAMER_CEF_SRC_CLIENT_H_


#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "boost/property_tree/ptree.hpp"
#include "include/cef_client.h"
#include "windows.h"  // NOLINT

#include "ncstreamer_cef/src/client/client_display_handler.h"
#include "ncstreamer_cef/src/client/client_life_span_handler.h"
#include "ncstreamer_cef/src/client/client_load_handler.h"
#include "ncstreamer_cef/src/streaming_service.h"
#include "ncstreamer_cef/src/streaming_service/streaming_service_types.h"


namespace ncstreamer {
class Client : public CefClient {
 public:
  Client(
      HINSTANCE instance,
      bool hides_settings,
      const std::wstring &video_quality,
      bool shows_sources_all,
      const std::vector<std::string> &sources,
      const std::wstring &locale,
      const StreamingServiceTagMap &tag_ids,
      const std::wstring &designated_user,
      const boost::property_tree::ptree &device_settings);

  virtual ~Client();

  CefRefPtr<CefDisplayHandler> GetDisplayHandler() override;
  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override;
  CefRefPtr<CefLoadHandler> GetLoadHandler() override;

  bool OnProcessMessageReceived(
      CefRefPtr<CefBrowser> browser,
      CefProcessId source_process,
      CefRefPtr<CefProcessMessage> message) override;

  const CefRefPtr<CefBrowser> &GetMainBrowser() const;

 private:
  bool OnRenderProcessMessageReceived(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefProcessMessage> message);

  bool OnRenderProcessEvent(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefProcessMessage> message);

  using OnInitialized = std::function<void()>;

  void InitializeService(const OnInitialized on_initialized);

  using CommandArgumentMap = std::unordered_map<std::string, std::string>;

  static CommandArgumentMap ParseVariables(const std::string &query);

  void OnCommand(const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);

  // command handlers
  using CommandHandler = std::function<void(
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
  void OnCommandStreamingSetUp(
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
  void OnCommandSettingsMicSearch(
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
  void OnCommandSettingsWebcamSearch(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandSettingsWebcamOn(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandSettingsWebcamOff(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandSettingsWebcamSizeUpdate(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandSettingsWebcamPositionUpdate(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandSettingsChromaKeyOn(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandSettingsChromaKeyOff(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandSettingsChromaKeyColorUpdate(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser);
  void OnCommandSettingsChromaKeySimilarityUpdate(
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
  void OnCommandStorageStreamServerUpdate(
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
  const StreamingServiceTagMap tag_ids_;
  const std::wstring designated_user_;
  CefRefPtr<ClientDisplayHandler> display_handler_;
  CefRefPtr<ClientLifeSpanHandler> life_span_handler_;
  CefRefPtr<ClientLoadHandler> load_handler_;

  IMPLEMENT_REFCOUNTING(Client);
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_CLIENT_H_
