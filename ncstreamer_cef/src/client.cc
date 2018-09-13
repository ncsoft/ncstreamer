/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/client.h"

#include <cassert>
#include <codecvt>
#include <locale>
#include <regex>  // NOLINT
#include <utility>

#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

#include "Shellapi.h"  // NOLINT
#include "Shlwapi.h"  // NOLINT

#include "ncstreamer_cef/src/designated_user.h"
#include "ncstreamer_cef/src/js_executor.h"
#include "ncstreamer_cef/src/local_storage.h"
#include "ncstreamer_cef/src/remote_server.h"
#include "ncstreamer_cef/src/obs.h"
#include "ncstreamer_cef/src/obs/obs_source_info.h"
#include "ncstreamer_cef/src/render_process_message_types.h"


namespace ncstreamer {
Client::Client(
    HINSTANCE instance,
    bool hides_settings,
    const std::wstring &video_quality,
    bool shows_sources_all,
    const std::vector<std::string> &sources,
    const std::wstring &locale,
    const StreamingServiceTagMap &tag_ids,
    const std::wstring &designated_user,
    const boost::property_tree::ptree &device_settings,
    const uint16_t &remote_port,
    const std::wstring &location)
    : locale_{locale},
      tag_ids_{tag_ids},
      designated_user_{designated_user},
      remote_port_{remote_port},
      location_{location},
      display_handler_{new ClientDisplayHandler{}},
      life_span_handler_{new ClientLifeSpanHandler{instance}},
      load_handler_{new ClientLoadHandler{life_span_handler_,
                                          hides_settings,
                                          video_quality,
                                          shows_sources_all,
                                          sources,
                                          device_settings}} {
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


void Client::InitializeService(const OnInitialized on_initialized) {
  ncstreamer::Obs::SetUp();
  ncstreamer::StreamingService::SetUp(tag_ids_);
  ncstreamer::DesignatedUser::SetUp(designated_user_);
  ncstreamer::RemoteServer::SetUp(GetMainBrowser());
  bool started = ncstreamer::RemoteServer::Get()->Start(remote_port_);
  on_initialized(started);
}


void Client::ShutdownService() {
  ncstreamer::RemoteServer::ShutDown();
  ncstreamer::StreamingService::ShutDown();
  ncstreamer::Obs::ShutDown();
}


Client::CommandArgumentMap
Client::ParseVariables(const std::string &query) {
  CommandArgumentMap args;
  static const std::regex kQueryPattern{R"(([\w+%]+)=([^,]*))"};

  auto begin = std::sregex_iterator(query.begin(), query.end(), kQueryPattern);
  auto end = std::sregex_iterator();

  for (std::sregex_iterator i = begin; i != end; ++i) {
    const auto &matches = *i;
    args.emplace(matches[1], matches[2]);
  }

  return std::move(args);
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
      {"streaming/set_up",
       std::bind(&This::OnCommandStreamingSetUp, this,
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
      {"settings/mic/search",
       std::bind(&This::OnCommandSettingsMicSearch, this,
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
      {"settings/webcam/search",
       std::bind(&This::OnCommandSettingsWebcamSearch, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"settings/webcam/on",
       std::bind(&This::OnCommandSettingsWebcamOn, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"settings/webcam/off",
       std::bind(&This::OnCommandSettingsWebcamOff, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"settings/webcam/size/update",
       std::bind(&This::OnCommandSettingsWebcamSizeUpdate, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"settings/webcam/position/update",
       std::bind(&This::OnCommandSettingsWebcamPositionUpdate, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"settings/chroma_key/on",
       std::bind(&This::OnCommandSettingsChromaKeyOn, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"settings/chroma_key/off",
       std::bind(&This::OnCommandSettingsChromaKeyOff, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"settings/chroma_key/color/update",
       std::bind(&This::OnCommandSettingsChromaKeyColorUpdate, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"settings/chroma_key/similarity/update",
       std::bind(&This::OnCommandSettingsChromaKeySimilarityUpdate, this,
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
      {"storage/stream_server/update",
       std::bind(&This::OnCommandStorageStreamServerUpdate, this,
           std::placeholders::_1,
           std::placeholders::_2,
           std::placeholders::_3)},
      {"storage/youtubePrivacy/update",
       std::bind(&This::OnCommandStorageYouTubePrivacyUpdate, this,
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
  ShutdownService();
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
  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  const auto &location = converter.to_bytes(location_);

  StreamingService::Get()->LogIn(
      service_provider,
      browser->GetHost()->GetWindowHandle(),
      locale_,
      [browser, cmd](const std::string &error) {
    JsExecutor::Execute(browser, "cef.onResponse", cmd,
        JsExecutor::StringPairVector{{"error", error}});
  }, [browser, cmd, location](
      const std::string &id,
      const std::string &access_token,
      const std::string &user_name,
      const std::vector<StreamingServiceProvider::UserPage> &user_pages,
      const std::vector<
          StreamingServiceProvider::StreamServer> &stream_servers) {
    std::vector<boost::property_tree::ptree> tree_pages;
    for (const auto &page : user_pages) {
      tree_pages.emplace_back(page.ToTree());
    }

    std::vector<boost::property_tree::ptree> tree_servers;
    for (const auto &server : stream_servers) {
      tree_servers.emplace_back(server.ToTree());
    }

    boost::property_tree::ptree arg;
    arg.add("error", "");
    arg.add("_id", id);
    arg.add("accessToken", access_token);
    arg.add("userName", user_name);
    arg.add_child("userPages", JsExecutor::ToPtree(tree_pages));
    arg.add_child("streamServers", JsExecutor::ToPtree(tree_servers));
    arg.add("userPage", LocalStorage::Get()->GetUserPage());
    arg.add("privacy", LocalStorage::Get()->GetPrivacy());
    arg.add("youtubePrivacy", LocalStorage::Get()->GetYouTubePrivacy());
    arg.add("streamServer", LocalStorage::Get()->GetStreamServer());
    arg.add("description", LocalStorage::Get()->GetDescription());
    arg.add("location", location);

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


void Client::OnCommandStreamingSetUp(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  if (::CefCurrentlyOn(TID_FILE) == true) {
    return;
  }

  ::CefPostTask(TID_FILE,
      base::Bind(&Client::InitializeService, base::Unretained(this),
          [cmd, browser, this](const bool &success) {
    assert(success);
    load_handler_->UpdateSourcesPeriodically(1000);

    JsExecutor::Execute(browser, "cef.onResponse", cmd,
        JsExecutor::StringPairVector{{"error", ""}});
  }));
}


void Client::OnCommandStreamingStart(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  auto source_i = args.find("source");
  auto stream_server_i = args.find("streamServer");
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
  const std::string &stream_server = stream_server_i->second;
  const std::string &user_page = user_page_i->second;
  const std::string &privacy = privacy_i->second;
  const std::string &title = title_i->second;
  const std::string &description = description_i->second;

  if (source.empty() == true) {
    assert(false);
    return;
  }

  LocalStorage::Get()->SetDescription(description);
  ObsSourceInfo source_info{source};
  StreamingService::Get()->PostLiveVideo(
      stream_server,
      user_page,
      privacy,
      title,
      description,
      source_info.title(),
      [browser, cmd](const std::string &error) {
    JsExecutor::Execute(browser, "cef.onResponse", cmd,
        JsExecutor::StringPairVector{{"error", error}});
  }, [browser, cmd, source](const std::string &service_provider,
                            const std::string &stream_server,
                            const std::string &stream_key,
                            const std::string &video_id,
                            const std::string &post_url) {
    bool result = Obs::Get()->StartStreaming(
        source,
        service_provider,
        stream_server,
        stream_key,
        [browser,
         cmd,
         service_provider,
         stream_server,
         stream_key,
         video_id,
         post_url]() {
      JsExecutor::Execute(browser, "cef.onResponse", cmd,
          JsExecutor::StringPairVector{
              {"error", ""},
              {"serviceProvider", service_provider},
              {"streamUrl", stream_server + stream_key},
              {"videoId", video_id},
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
  StreamingService::Get()->StopLiveVideo();

  Obs::Get()->StopStreaming([browser, cmd]() {
    JsExecutor::Execute(browser, "cef.onResponse", cmd,
        JsExecutor::StringPairVector{{"error", ""}});
  });
}


void Client::OnCommandSettingsMicSearch(
      const std::string &cmd,
      const CommandArgumentMap &args,
      CefRefPtr<CefBrowser> browser) {
  std::unordered_map<std::string /*id*/, std::string /*name*/>
      mic_devices = Obs::Get()->SearchMicDevices();

  std::vector<boost::property_tree::ptree> tree_webcams;
  for (const auto &mic : mic_devices) {
    boost::property_tree::ptree tree;
    tree.put("id", mic.first);
    tree.put("name", mic.second);
    tree_webcams.emplace_back(tree);
  }

  boost::property_tree::ptree arg;
  arg.add("error", "");
  arg.add_child("micList", JsExecutor::ToPtree(tree_webcams));

  JsExecutor::Execute(browser, "cef.onResponse", cmd, arg);
}


void Client::OnCommandSettingsMicOn(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  auto device_id_i = args.find("deviceId");
  auto volume_i = args.find("volume");
  if (device_id_i == args.end() ||
      volume_i == args.end()) {
    assert(false);
    return;
  }

  const std::string &device_id = device_id_i->second;
  float volume{0.0};
  try {
    volume = std::stof(volume_i->second);
  } catch (...) {
    assert(false);
    return;
  }

  std::string error{};
  bool result = Obs::Get()->TurnOnMic(device_id, &error);
  if (!result) {
    if (error.empty()) {
      error = "failed to turn mic on";
    }
    JsExecutor::Execute(browser, "cef.onResponse", cmd,
        JsExecutor::StringPairVector{{"error", error},
                                     {"volume", volume_i->second}});
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
  } catch (...) {
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


void Client::OnCommandSettingsWebcamSearch(
  const std::string &cmd,
  const CommandArgumentMap &args,
  CefRefPtr<CefBrowser> browser) {
  std::vector<std::string> webcams = Obs::Get()->FindAllWebcamDevices();

  std::vector<boost::property_tree::ptree> tree_webcams;
  for (const auto &webcam : webcams) {
    boost::property_tree::ptree tree;
    tree.put("id", webcam);
    tree_webcams.emplace_back(tree);
  }

  boost::property_tree::ptree arg;
  arg.add("error", "");
  arg.add_child("webcamList", JsExecutor::ToPtree(tree_webcams));

  JsExecutor::Execute(browser, "cef.onResponse", cmd, arg);
}


void Client::OnCommandSettingsWebcamOn(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  auto device_id_i = args.find("deviceId");
  auto normal_width_i = args.find("normalWidth");
  auto normal_height_i = args.find("normalHeight");
  auto normal_x_i = args.find("normalX");
  auto normal_y_i = args.find("normalY");
  if (device_id_i == args.end() ||
      normal_width_i == args.end() ||
      normal_height_i == args.end() ||
      normal_x_i == args.end() ||
      normal_y_i == args.end()) {
    assert(false);
    return;
  }

  const std::string &device_id = device_id_i->second;
  float normal_width{0.0};
  float normal_height{0.0};
  float normal_x{0.0};
  float normal_y{0.0};
  try {
    normal_width = std::stof(normal_width_i->second);
    normal_height = std::stof(normal_height_i->second);
    normal_x = std::stof(normal_x_i->second);
    normal_y = std::stof(normal_y_i->second);
  } catch (...) {
    assert(false);
    return;
  }

  std::string error{};
  bool result = Obs::Get()->TurnOnWebcam(device_id, &error);
  if (!result) {
    if (error.empty()) {
      error = "failed to turn webcam on";
    }
    JsExecutor::Execute(browser, "cef.onResponse", cmd,
        JsExecutor::StringPairVector{{"error", error}});
    return;
  }

  if (Obs::Get()->UpdateWebcamSize(normal_width, normal_height) == false) {
    error = "webcam size error";
  }

  if (Obs::Get()->UpdateWebcamPosition(normal_x, normal_y) == false) {
    error = "webcam position error";
  }

  JsExecutor::Execute(browser, "cef.onResponse", cmd,
      JsExecutor::StringPairVector{{"error", error}});
}


void Client::OnCommandSettingsWebcamOff(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  bool result = Obs::Get()->TurnOffWebcam();
  if (!result) {
    std::string error{"failed to turn webcam off"};
    JsExecutor::Execute(browser, "cef.onResponse", cmd,
        JsExecutor::StringPairVector{{"error", error}});
    return;
  }

  JsExecutor::Execute(browser, "cef.onResponse", cmd,
      JsExecutor::StringPairVector{{"error", ""}});
}


void Client::OnCommandSettingsWebcamSizeUpdate(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  auto normal_width_i = args.find("normalWidth");
  auto normal_height_i = args.find("normalHeight");
  if (normal_width_i == args.end() ||
      normal_height_i == args.end()) {
    assert(false);
    return;
  }

  float normal_width{0.0};
  float normal_height{0.0};
  try {
    normal_width = std::stof(normal_width_i->second);
    normal_height = std::stof(normal_height_i->second);
  } catch (...) {
    assert(false);
    return;
  }

  std::string error{};
  bool result = Obs::Get()->UpdateWebcamSize(normal_width, normal_height);
  if (!result) {
    error = "failed to update webcam size";
  }
  JsExecutor::Execute(browser, "cef.onResponse", cmd,
      JsExecutor::StringPairVector{{"error", error},
                                   {"normalWidth", normal_width_i->second},
                                   {"normalHeight", normal_height_i->second}});
}


void Client::OnCommandSettingsWebcamPositionUpdate(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  auto normal_x_i = args.find("normalX");
  auto normal_y_i = args.find("normalY");
  if (normal_x_i == args.end() ||
      normal_y_i == args.end()) {
    assert(false);
    return;
  }

  float normal_x{0.0};
  float normal_y{0.0};
  try {
    normal_x = std::stof(normal_x_i->second);
    normal_y = std::stof(normal_y_i->second);
  } catch (...) {
    assert(false);
    return;
  }

  std::string error{};
  bool result = Obs::Get()->UpdateWebcamPosition(normal_x, normal_y);
  if (!result) {
    error = "failed to update webcam position";
  }
  JsExecutor::Execute(browser, "cef.onResponse", cmd,
      JsExecutor::StringPairVector{{"error", error},
                                   {"normalX", normal_x_i->second},
                                   {"normalY", normal_y_i->second}});
}


void Client::OnCommandSettingsChromaKeyOn(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  auto color_i = args.find("color");
  auto similarity_i = args.find("similarity");
  if (color_i == args.end() ||
      similarity_i == args.end()) {
    assert(false);
    return;
  }

  uint32_t color{0};
  int similarity{0};
  try {
    color = std::stoul(color_i->second);
    similarity = std::stoi(similarity_i->second);
  } catch (...) {
    assert(false);
    return;
  }

  bool result = Obs::Get()->TurnOnChromaKey(color, similarity);
  if (!result) {
    std::string error{"failed to turn chroma key on"};
    JsExecutor::Execute(browser, "cef.onResponse", cmd,
        JsExecutor::StringPairVector{{"error", error}});
    return;
  }

  JsExecutor::Execute(browser, "cef.onResponse", cmd,
      JsExecutor::StringPairVector{{"error", ""}});
}


void Client::OnCommandSettingsChromaKeyOff(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  bool result = Obs::Get()->TurnOffChromaKey();
  if (!result) {
    std::string error{"failed to turn chroma key off"};
    JsExecutor::Execute(browser, "cef.onResponse", cmd,
        JsExecutor::StringPairVector{{"error", error}});
    return;
  }

  JsExecutor::Execute(browser, "cef.onResponse", cmd,
      JsExecutor::StringPairVector{{"error", ""}});
}


void Client::OnCommandSettingsChromaKeyColorUpdate(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  auto color_i = args.find("color");
  if (color_i == args.end()) {
    assert(false);
    return;
  }

  uint32_t color{0};
  try {
    color = std::stoul(color_i->second);
  } catch (...) {
    assert(false);
    return;
  }

  bool result = Obs::Get()->UpdateChromaKeyColor(color);
  if (!result) {
    std::string error{"failed to update chroma key color"};
    JsExecutor::Execute(browser, "cef.onResponse", cmd,
        JsExecutor::StringPairVector{{"error", error}});
    return;
  }

  JsExecutor::Execute(browser, "cef.onResponse", cmd,
      JsExecutor::StringPairVector{{"error", ""}});
}


void Client::OnCommandSettingsChromaKeySimilarityUpdate(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
  auto similarity_i = args.find("similarity");
  if (similarity_i == args.end()) {
    assert(false);
    return;
  }

  int similarity{0};
  try {
    similarity = std::stoi(similarity_i->second);
  } catch (...) {
    assert(false);
    return;
  }

  bool result = Obs::Get()->UpdateChromaKeyColor(similarity);
  if (!result) {
    std::string error{"failed to update chroma key similarity"};
    JsExecutor::Execute(browser, "cef.onResponse", cmd,
        JsExecutor::StringPairVector{{"error", error}});
    return;
  }

  JsExecutor::Execute(browser, "cef.onResponse", cmd,
      JsExecutor::StringPairVector{{"error", ""}});
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
  } catch (...) {
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


void Client::OnCommandStorageStreamServerUpdate(
  const std::string &cmd,
  const CommandArgumentMap &args,
  CefRefPtr<CefBrowser> /*browser*/) {
  auto stream_server_i = args.find("streamServer");
  if (stream_server_i == args.end()) {
    assert(false);
    return;
  }

  const std::string &stream_server = stream_server_i->second;

  if (stream_server.empty() == true) {
    assert(false);
    return;
  }

  LocalStorage::Get()->SetStreamServer(stream_server);
}


void Client::OnCommandStorageYouTubePrivacyUpdate(
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

  LocalStorage::Get()->SetYouTubePrivacy(privacy);
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
  } catch (...) {
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
  auto id_i = args.find("id");
  auto video_id_i = args.find("videoId");
  auto access_token_i = args.find("token");
  if (request_key_i == args.end() ||
      error_i == args.end() ||
      source_i == args.end() ||
      user_page_i == args.end() ||
      privacy_i == args.end() ||
      description_i == args.end() ||
      mic_i == args.end() ||
      service_provider_i == args.end() ||
      stream_url_i == args.end() ||
      post_url_i == args.end() ||
      id_i == args.end() ||
      video_id_i == args.end() ||
      access_token_i == args.end()) {
    assert(false);
    return;
  }

  int request_key{0};
  try {
    request_key = std::stoi(request_key_i->second);
  } catch (...) {
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
  const std::string &id = id_i->second;
  const std::string &video_id = video_id_i->second;
  const std::string &access_token = access_token_i->second;

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
      post_url,
      id,
      video_id,
      access_token);
}


void Client::OnCommandRemoteStop(
    const std::string &cmd,
    const CommandArgumentMap &args,
    CefRefPtr<CefBrowser> browser) {
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
  } catch (...) {
  }

  const std::string &error = error_i->second;
  const std::string &source = source_i->second;

  RemoteServer::Get()->NotifyStreamingStop(
      request_key,
      error,
      source);

  JsExecutor::Execute(browser, "cef.onResponse", cmd,
      JsExecutor::StringPairVector{{"error", ""}});
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
  } catch (...) {
  }

  const std::string &error = error_i->second;

  RemoteServer::Get()->NotifySettingsQualityUpdate(
      request_key,
      error);
}
}  // namespace ncstreamer
