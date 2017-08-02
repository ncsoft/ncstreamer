/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/client.h"

#include <cassert>
#include <codecvt>
#include <locale>
#include <regex>  // NOLINT
#include <utility>

#include "boost/property_tree/ptree.hpp"
#include "include/wrapper/cef_helpers.h"

#include "Shellapi.h"  // NOLINT
#include "Shlwapi.h"  // NOLINT

#include "ncstreamer_cef/src/js_executor.h"
#include "ncstreamer_cef/src/local_storage.h"
#include "ncstreamer_cef/src/remote_server.h"
#include "ncstreamer_cef/src/obs.h"
#include "ncstreamer_cef/src/obs/obs_source_info.h"
#include "ncstreamer_cef/src/render_process_message_types.h"
#include "ncstreamer_cef/src/streaming_service.h"


namespace ncstreamer {
Client::Client(
    HINSTANCE instance,
    bool hides_settings,
    const std::wstring &video_quality,
    bool shows_sources_all,
    const std::vector<std::string> &sources,
    const std::wstring &locale)
    : locale_{locale},
      display_handler_{new ClientDisplayHandler{}},
      life_span_handler_{new ClientLifeSpanHandler{instance}},
      load_handler_{new ClientLoadHandler{life_span_handler_,
                                          hides_settings,
                                          video_quality,
                                          shows_sources_all,
                                          sources}} {
}


Client::~Client() {
}


CefRefPtr<CefDisplayHandler> Client::GetDisplayHandler() {
  return display_handler_;
}


CefRefPtr<CefLifeSpanHandler> Client::GetLifeSpanHandler() {
  return life_span_handler_;
}


CefRefPtr<CefLoadHandler> Client::GetLoadHandler() {
  return load_handler_;
}


bool Client::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) {
  CEF_REQUIRE_UI_THREAD();

  if (!message->IsValid()) {
    return false;
  }

  switch (source_process) {
    case PID_RENDERER: {
      return OnRenderProcessMessageReceived(browser, message);
    }
    default: {
      DCHECK(false);
      return false;
    }
  }
}


const CefRefPtr<CefBrowser> &Client::GetMainBrowser() const {
  return life_span_handler_->main_browser();
}


bool Client::OnRenderProcessMessageReceived(
  CefRefPtr<CefBrowser> browser,
  CefRefPtr<CefProcessMessage> message) {
  CEF_REQUIRE_UI_THREAD();
  CefString msg_name = message->GetName();
  if (msg_name == RenderProcessMessage::kEvent) {
    return OnRenderProcessEvent(browser, message);
  }

  DCHECK(false);
  return false;
}


bool Client::OnRenderProcessEvent(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefProcessMessage> message) {
  auto args = message->GetArgumentList();
  std::string type = args->GetString(0);
  std::string variables = args->GetString(1);
  OnCommand(type, ParseVariables(variables), browser);
  return true;
}


Client::CommandArgumentMap
Client::ParseVariables(const std::string &query) {
  CommandArgumentMap args;
  static const std::regex kQueryPattern{R"(([\w+%]+)=([^,]*))"};

  auto begin = std::sregex_iterator(query.begin(), query.end(), kQueryPattern);
  auto end = std::sregex_iterator();

  for (std::sregex_iterator i = begin; i != end; ++i) {
    const auto &matches = *i;
    args.emplace(matches[1], DecodeUri(matches[2]));
  }

  return std::move(args);
}


std::string Client::DecodeUri(const std::string &enc_string) {
  static const std::size_t kMaxSize{2048};

  if (enc_string.size() >= kMaxSize - 1) {
    return enc_string;
  }

  char buf[kMaxSize];
  std::strncpy(buf, enc_string.c_str(), enc_string.size() + 1);

  HRESULT result = ::UrlUnescapeA(buf, NULL, NULL, URL_UNESCAPE_INPLACE);
  if (result != S_OK) {
    return enc_string;
  }

  return std::move(std::string{buf});
}


void Client::OnCommand(const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  using This = Client;
  static const std::unordered_map<std::string/*command*/,
                                  CommandHandler> kCommandHandlers{
      {"window/close",
       std::bind(&This::OnCommandWindowClose, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"window/minimize",
       std::bind(&This::OnCommandWindowMinimize, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"external_browser/pop_up",
       std::bind(&This::OnCommandExternalBrowserPopUp, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"service_provider/log_in",
       std::bind(&This::OnCommandServiceProviderLogIn, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"service_provider/log_out",
       std::bind(&This::OnCommandServiceProviderLogOut, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"streaming/start",
       std::bind(&This::OnCommandStreamingStart, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"streaming/stop",
       std::bind(&This::OnCommandStreamingStop, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"settings/mic/on",
       std::bind(&This::OnCommandSettingsMicOn, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"settings/mic/off",
       std::bind(&This::OnCommandSettingsMicOff, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"settings/mic/volume/update",
       std::bind(&This::OnCommandSettingsMicVolumeUpdate, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"settings/video_quality/update",
       std::bind(&This::OnCommandSettingsVideoQualityUpdate, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"storage/user_page/update",
       std::bind(&This::OnCommandStorageUserPageUpdate, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"storage/privacy/update",
       std::bind(&This::OnCommandStoragePrivacyUpdate, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"remote/status",
       std::bind(&This::OnCommandRemoteStatus, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"remote/start",
       std::bind(&This::OnCommandRemoteStart, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"remote/stop",
       std::bind(&This::OnCommandRemoteStop, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"remote/quality/update",
       std::bind(&This::OnCommandRemoteQualityUpdate, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)}};

  auto i = kCommandHandlers.find(cmd);
  if (i == kCommandHandlers.end()) {
    assert(false);
    return;
  }

  i->second(cmd, args, browser);
}


void Client::OnCommandWindowClose(
    const std::string &/*cmd*/,
    const CommandArgumentMap &/*args*/,
    CefRefPtr<CefBrowser> browser) {
  browser->GetHost()->CloseBrowser(true);
}


void Client::OnCommandWindowMinimize(
    const std::string &/*cmd*/,
    const CommandArgumentMap &/*args*/,
    CefRefPtr<CefBrowser> browser) {
  HWND wnd{browser->GetHost()->GetWindowHandle()};
  ::ShowWindow(wnd, SW_MINIMIZE);
}


void Client::OnCommandExternalBrowserPopUp(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  auto uri_i = args.find("uri");
  if (uri_i == args.end()) {
    assert(false);
    return;
  }

  const std::string &uri = uri_i->second;

  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

  ::ShellExecute(NULL, L"open", converter.from_bytes(uri).c_str(),
      NULL, NULL, SW_SHOWNORMAL);
}


void Client::OnCommandServiceProviderLogIn(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  auto provider_i = args.find("serviceProvider");
  if (provider_i == args.end()) {
    assert(false);
    return;
  }

  const std::string &service_provider = provider_i->second;

  StreamingService::Get()->LogIn(
      service_provider,
      browser->GetHost()->GetWindowHandle(),
      locale_,
      [](const std::string &/*error*/) {
    // nothing to do.
  }, [browser, cmd](
      const std::string &user_name,
      const std::string &user_link,
      const std::vector<StreamingServiceProvider::UserPage> &user_pages) {
    std::vector<boost::property_tree::ptree> tree_pages;
    for (const auto &page : user_pages) {
      tree_pages.emplace_back(page.ToTree());
    }

    boost::property_tree::ptree arg;
    arg.add("userName", user_name);
    arg.add("userLink", user_link);
    arg.add_child("userPages", JsExecutor::ToPtree(tree_pages));
    arg.add("userPage", LocalStorage::Get()->GetUserPage());
    arg.add("privacy", LocalStorage::Get()->GetPrivacy());

    JsExecutor::Execute(browser, "cef.onResponse", cmd, arg);
  });
}


void Client::OnCommandServiceProviderLogOut(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  auto provider_i = args.find("serviceProvider");
  if (provider_i == args.end()) {
    assert(false);
    return;
  }

  const std::string &service_provider = provider_i->second;

  StreamingService::Get()->LogOut(
      service_provider,
      [browser, cmd](const std::string &error) {
    JsExecutor::Execute(browser, "cef.onResponse", cmd,
        JsExecutor::StringPairVector{{"error", error}});
  }, [browser, cmd]() {
    JsExecutor::Execute(browser, "cef.onResponse", cmd,
        JsExecutor::StringPairVector{{"error", ""}});
  });
}


void Client::OnCommandStreamingStart(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  auto source_i = args.find("source");
  auto user_page_i = args.find("userPage");
  auto privacy_i = args.find("privacy");
  auto title_i = args.find("title");
  auto description_i = args.find("description");
  if (source_i == args.end() ||
    user_page_i == args.end() ||
    privacy_i == args.end() ||
    title_i == args.end() ||
    description_i == args.end()) {
    assert(false);
    return;
  }

  const std::string &source = source_i->second;
  const std::string &user_page = user_page_i->second;
  const std::string &privacy = privacy_i->second;
  const std::string &title = title_i->second;
  const std::string &description = description_i->second;

  if (source.empty() == true ||
    user_page.empty() == true ||
    privacy.empty() == true) {
    assert(false);
    return;
  }

  ObsSourceInfo source_info{source};

  StreamingService::Get()->PostLiveVideo(
      user_page,
      privacy,
      title,
      description,
      source_info.title(),
      [browser, cmd](const std::string &error) {
    JsExecutor::Execute(browser, "cef.onResponse", cmd,
        JsExecutor::StringPairVector{{"error", error}});
  }, [browser, cmd, source](const std::string &service_provider,
    const std::string &stream_url,
    const std::string &post_url) {
    bool result = Obs::Get()->StartStreaming(
        source,
        service_provider,
        stream_url,
        [browser, cmd, service_provider, stream_url, post_url]() {
      JsExecutor::Execute(browser, "cef.onResponse", cmd,
          JsExecutor::StringPairVector{
              {"error", ""},
              {"serviceProvider", service_provider},
              {"streamUrl", stream_url},
              {"postUrl", post_url}});
    });
    if (result == false) {
      JsExecutor::Execute(browser, "cef.onResponse", cmd,
          JsExecutor::StringPairVector{{"error", "obs internal"}});
    }
  });
}


void Client::OnCommandStreamingStop(
    const std::string &cmd,
    const CommandArgumentMap &/*args*/,
    CefRefPtr<CefBrowser> browser) {
  Obs::Get()->StopStreaming([browser, cmd]() {
    JsExecutor::Execute(browser, "cef.onResponse", cmd,
        JsExecutor::StringPairVector{{"error", ""}});
  });
}


void Client::OnCommandSettingsMicOn(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  std::string error{};
  bool result = Obs::Get()->TurnOnMic();
  if (!result) {
    error = "turn on after start streaming";
    JsExecutor::Execute(browser, "cef.onResponse", cmd,
        JsExecutor::StringPairVector{{"error", error},
                                     {"volume", ""}});
    return;
  }

  auto volume_i = args.find("volume");
  if (volume_i == args.end()) {
    assert(false);
    return;
  }

  float volume{0.0};
  try {
    volume = std::stof(volume_i->second);
  }
  catch (...) {
    assert(false);
    return;
  }
  result = Obs::Get()->UpdateMicVolume(volume);
  if (!result) {
    error = "update volume error";
  }
  JsExecutor::Execute(browser, "cef.onResponse", cmd,
      JsExecutor::StringPairVector{{"error", error},
                                   {"volume", volume_i->second}});
}


void Client::OnCommandSettingsMicOff(
    const std::string &cmd,
    const CommandArgumentMap &/*args*/,
    CefRefPtr<CefBrowser> browser) {
  std::string error{};
  bool result = Obs::Get()->TurnOffMic();
  if (!result) {
    error = "turn off error";
  }
  JsExecutor::Execute(browser, "cef.onResponse", cmd,
      JsExecutor::StringPairVector{{"error", error}});
}


void Client::OnCommandSettingsMicVolumeUpdate(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  auto volume_i = args.find("volume");
  if (volume_i == args.end()) {
    assert(false);
    return;
  }

  float volume{0.0};
  try {
    volume = std::stof(volume_i->second);
  }
  catch (...) {
    assert(false);
    return;
  }
  std::string error{};
  bool result = Obs::Get()->UpdateMicVolume(volume);
  if (!result) {
    error = "update volume after turn on mic";
  }
  JsExecutor::Execute(browser, "cef.onResponse", cmd,
      JsExecutor::StringPairVector{{"error", error},
                                   {"volume", volume_i->second}});
}


void Client::OnCommandSettingsVideoQualityUpdate(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  auto width_i = args.find("width");
  auto height_i = args.find("height");
  auto fps_i = args.find("fps");
  auto bitrate_i = args.find("bitrate");
  if (width_i == args.end() ||
    height_i == args.end() ||
    fps_i == args.end() ||
    bitrate_i == args.end()) {
    assert(false);
    return;
  }

  uint32_t width{0};
  uint32_t height{0};
  uint32_t fps{0};
  uint32_t bitrate{0};
  try {
    width = std::stoul(width_i->second);
    height = std::stoul(height_i->second);
    fps = std::stoul(fps_i->second);
    bitrate = std::stoul(bitrate_i->second);
  }
  catch (...) {
  }

  if (width == 0 ||
    height == 0 ||
    fps == 0 ||
    bitrate == 0) {
    assert(false);
    return;
  }

  Obs::Get()->UpdateVideoQuality({width, height}, fps, bitrate);
  JsExecutor::Execute(browser, "cef.onResponse", cmd,
      JsExecutor::StringPairVector{{"error", ""}});
}


void Client::OnCommandStorageUserPageUpdate(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> /*browser*/) {
  auto user_page_i = args.find("userPage");
  if (user_page_i == args.end()) {
    assert(false);
    return;
  }

  const std::string &user_page = user_page_i->second;

  if (user_page.empty() == true) {
    assert(false);
    return;
  }

  LocalStorage::Get()->SetUserPage(user_page);
}


void Client::OnCommandStoragePrivacyUpdate(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> /*browser*/) {
  auto privacy_i = args.find("privacy");
  if (privacy_i == args.end()) {
    assert(false);
    return;
  }

  const std::string &privacy = privacy_i->second;

  if (privacy.empty() == true) {
    assert(false);
    return;
  }

  LocalStorage::Get()->SetPrivacy(privacy);
}


void Client::OnCommandRemoteStatus(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> /*browser*/) {
  auto request_key_i = args.find("requestKey");
  auto status_i = args.find("status");
  auto source_title_i = args.find("sourceTitle");
  auto user_name_i = args.find("userName");
  auto quality_i = args.find("quality");
  if (request_key_i == args.end() ||
    status_i == args.end() ||
    source_title_i == args.end() ||
    user_name_i == args.end() ||
    quality_i == args.end()) {
    assert(false);
    return;
  }

  int request_key{0};
  try {
    request_key = std::stoi(request_key_i->second);
  }
  catch (...) {
  }

  if (request_key == 0) {
    assert(false);
    return;
  }

  const std::string &status = status_i->second;
  const std::string &source_title = source_title_i->second;
  const std::string &user_name = user_name_i->second;
  const std::string &quality = quality_i->second;

  if (status.empty() == true ||
    quality.empty() == true) {
    assert(false);
    return;
  }

  RemoteServer::Get()->RespondStreamingStatus(
      request_key,
      status,
      source_title,
      user_name,
      quality);
}


void Client::OnCommandRemoteStart(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  auto request_key_i = args.find("requestKey");
  auto error_i = args.find("error");
  auto source_i = args.find("source");
  auto user_page_i = args.find("userPage");
  auto privacy_i = args.find("privacy");
  auto description_i = args.find("description");
  auto mic_i = args.find("mic");
  auto service_provider_i = args.find("serviceProvider");
  auto stream_url_i = args.find("streamUrl");
  auto post_url_i = args.find("postUrl");
  if (request_key_i == args.end() ||
    error_i == args.end() ||
    source_i == args.end() ||
    user_page_i == args.end() ||
    privacy_i == args.end() ||
    description_i == args.end() ||
    mic_i == args.end() ||
    service_provider_i == args.end() ||
    stream_url_i == args.end() ||
    post_url_i == args.end()) {
    assert(false);
    return;
  }

  int request_key{0};
  try {
    request_key = std::stoi(request_key_i->second);
  }
  catch (...) {
  }

  const std::string &error = error_i->second;
  const std::string &source = source_i->second;
  const std::string &user_page = user_page_i->second;
  const std::string &privacy = privacy_i->second;
  const std::string &description = description_i->second;
  const std::string &mic = mic_i->second;
  const std::string &service_provider = service_provider_i->second;
  const std::string &stream_url = stream_url_i->second;
  const std::string &post_url = post_url_i->second;

  RemoteServer::Get()->NotifyStreamingStart(
      request_key,
      error,
      source,
      user_page,
      privacy,
      description,
      mic,
      service_provider,
      stream_url,
      post_url);
  JsExecutor::Execute(browser, "cef.onResponse", cmd);
}


void Client::OnCommandRemoteStop(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> /*browser*/) {
  auto request_key_i = args.find("requestKey");
  auto error_i = args.find("error");
  auto source_i = args.find("source");
  if (request_key_i == args.end() ||
    error_i == args.end() ||
    source_i == args.end()) {
    assert(false);
    return;
  }

  int request_key{0};
  try {
    request_key = std::stoi(request_key_i->second);
  }
  catch (...) {
  }

  const std::string &error = error_i->second;
  const std::string &source = source_i->second;

  RemoteServer::Get()->NotifyStreamingStop(
      request_key,
      error,
      source);
}


void Client::OnCommandRemoteQualityUpdate(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> /*browser*/) {
  auto request_key_i = args.find("requestKey");
  auto error_i = args.find("error");
  if (request_key_i == args.end() ||
    error_i == args.end()) {
    assert(false);
    return;
  }

  int request_key{0};
  try {
    request_key = std::stoi(request_key_i->second);
  }
  catch (...) {
  }

  const std::string &error = error_i->second;

  RemoteServer::Get()->NotifySettingsQualityUpdate(
      request_key,
      error);
}
}  // namespace ncstreamer
