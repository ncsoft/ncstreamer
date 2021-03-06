/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/obs.h"

#include <cassert>
#include <codecvt>

#include "windows.h"  //NOLINT

#include "obs-studio/libobs/graphics/vec2.h"
#include "obs-studio/plugins/win-capture/graphics-hook-info.h"

#include "ncstreamer_cef/src/local_storage.h"
#include "ncstreamer_cef/src/obs/obs_source_info.h"
#include "ncstreamer_cef/src_imported/from_obs_studio_ui/obs-app.hpp"


namespace ncstreamer {
void Obs::SetUp() {
  assert(!static_instance);
  static_instance = new Obs{};
}


void Obs::ShutDown() {
  assert(static_instance);
  delete static_instance;
  static_instance = nullptr;
}


Obs *Obs::Get() {
  assert(static_instance);
  return static_instance;
}


std::vector<std::string> Obs::FindAllWindowsOnDesktop() {
  std::vector<std::string> titles;

  obs_source_t *source =
      obs_source_create("game_capture", "Game Capture", nullptr, nullptr);
  obs_properties_t *props = obs_source_properties(source);
  obs_property_t *prop = obs_properties_get(props, "window");

  int count = obs_property_list_item_count(prop);
  for (int i = 0; i < count; i++) {
    const char *val = obs_property_list_item_string(prop, i);
    if (strlen(val) != 0) {
      const std::string &decoded = DecodeObsString(val);
      titles.emplace_back(decoded);
    }
  }

  obs_properties_destroy(props);
  obs_source_release(source);
  return titles;
}


std::vector<std::string> Obs::FindAllWebcamDevices() {
  std::vector<std::string> titles;

  obs_source_t *source = obs_source_create(
      "dshow_input", "VideoCaptureDevice", nullptr, nullptr);
  obs_properties_t *props = obs_source_properties(source);
  obs_property_t *prop = obs_properties_get(props, "video_device_id");

  int count = obs_property_list_item_count(prop);
  for (int i = 0; i < count; i++) {
    const char *val = obs_property_list_item_string(prop, i);
    if (strlen(val) != 0) {
      const std::string &decoded = DecodeObsString(val);
      titles.emplace_back(decoded);
    }
  }

  obs_properties_destroy(props);
  obs_source_release(source);
  return titles;
}


bool Obs::StartStreaming(
    const std::string &source_info,
    const std::string &service_provider,
    const std::string &stream_server,
    const std::string &stream_key,
    const ObsOutput::OnStarted &on_streaming_started,
    const ObsOutput::OnStopped &on_streaming_timeout) {
  UpdateVideoSource(source_info);
  UpdateBaseResolution(source_info);

  ResetVideo();
  obs_encoder_set_audio(audio_encoder_, obs_get_audio());
  obs_encoder_set_video(video_encoder_, obs_get_video());

  UpdateCurrentService(service_provider, stream_server, stream_key);
  UpdateCurrentServiceEncoders(audio_bitrate_, video_bitrate_);

  return stream_output_->Start(
      audio_encoder_,
      video_encoder_,
      current_service_,
      on_streaming_started,
      on_streaming_timeout);
}


void Obs::StopStreaming(
    const ObsOutput::OnStopped &on_streaming_stopped) {
  ClearSceneItems();
  stream_output_->Stop(on_streaming_stopped);
}


std::unordered_map<std::string, std::string> Obs::SearchMicDevices() {
  obs_properties_t *input_props = obs_get_source_properties(
      "wasapi_input_capture");
  obs_property_t *prop = obs_properties_get(
      input_props, "device_id");
  size_t count = obs_property_list_item_count(prop);
  std::unordered_map<std::string, std::string> mic_map;
  for (size_t i = 0; i < count; i++) {
    const char *val = obs_property_list_item_string(prop, i);
    const char *name = obs_property_list_item_name(prop, i);
    mic_map.emplace(val, name);
  }
  obs_properties_destroy(input_props);
  return mic_map;
}


bool Obs::TurnOnMic(const std::string &device_id, std::string *const error) {
  obs_sceneitem_t *item = obs_scene_find_source(scene_, "Game Capture");
  if (item == nullptr) {
    return false;
  }

  std::unordered_map<std::string, std::string> mic_devices = SearchMicDevices();
  const auto &iterator = mic_devices.find(device_id);
  if (iterator == mic_devices.end()) {
    *error = "no device ID";
    return false;
  }

  obs_source_t *source = obs_get_output_source(3);
  if (source == nullptr) {
    obs_data_t *settings = obs_data_create();
    obs_data_set_string(settings, "device_id", device_id.c_str());
    source = obs_source_create(
        "wasapi_input_capture", "Mic/Aux", settings, nullptr);
    obs_data_release(settings);
    obs_set_output_source(3, source);
  } else {
    obs_data_t *settings = obs_source_get_settings(source);
    obs_data_set_string(settings, "device_id", device_id.c_str());
    obs_source_update(source, settings);
    obs_data_release(settings);
  }
  obs_source_release(source);
  return true;
}


bool Obs::TurnOffMic() {
  obs_set_output_source(3, nullptr);
  return true;
}


bool Obs::UpdateMicVolume(const float &volume) {
  obs_source_t *source = obs_get_output_source(3);
  if (!source) {
    return false;
  }
  obs_source_set_volume(source, volume);
  obs_source_release(source);
  return true;
}


bool Obs::TurnOnWebcam(
    const std::string &device_id, std::string *const error) {
  obs_sceneitem_t *game_item = obs_scene_find_source(scene_, "Game Capture");
  if (game_item == nullptr) {
    return false;
  }

  if (CheckDeviceId(device_id) == false) {
    *error = "no device ID";
    return false;
  }

  obs_sceneitem_t *item = obs_scene_find_source(scene_, "Video Capture Device");
  if (item == nullptr) {
    obs_data_t *settings = obs_data_create();
    obs_data_set_string(settings, "video_device_id", device_id.c_str());
    obs_data_set_int(settings, "res_type", 0);  // Type: Preferred(0), Custom(1)
    obs_source_t *source = obs_source_create(
        "dshow_input", "Video Capture Device", settings, nullptr);
    obs_data_release(settings);
    obs_scene_atomic_update(scene_, Obs::AddSourceToScene, source);
    obs_source_release(source);
  } else {
    obs_source_t *source = obs_get_source_by_name("Video Capture Device");
    obs_data_t *settings = obs_source_get_settings(source);
    obs_data_set_string(settings, "video_device_id", device_id.c_str());
    obs_source_update(source, settings);
    obs_data_release(settings);
    obs_source_release(source);
  }

  return true;
}


bool Obs::TurnOffWebcam() {
  obs_sceneitem_t *item = obs_scene_find_source(scene_, "Video Capture Device");
  if (item == nullptr) {
    return false;
  }
  obs_sceneitem_remove(item);
  return true;
}


bool Obs::UpdateWebcamSize(const float &normal_x, const float &normal_y) {
  obs_sceneitem_t *item = obs_scene_find_source(scene_, "Video Capture Device");
  if (item == nullptr) {
    return false;
  }

  obs_source_t *source = obs_sceneitem_get_source(item);
  int width{0};
  int height{0};
  for (int i = 0; i < 50; ++i) {  // setting webcam source size timing issue.
    width = obs_source_get_width(source);
    height = obs_source_get_height(source);
    if (width != 0 && height != 0) {
      break;
    }
    Sleep(100);
  }

  vec2 from_size{static_cast<float>(width), static_cast<float>(height)};
  vec2 to_size{base_size_.width() * normal_x, base_size_.height() * normal_y};
  vec2 scale;
  vec2_div(&scale, &to_size, &from_size);
  obs_sceneitem_set_scale(item, &scale);
  return true;
}


bool Obs::UpdateWebcamPosition(const float &normal_x, const float &normal_y) {
  obs_sceneitem_t *item = obs_scene_find_source(scene_, "Video Capture Device");
  if (item == nullptr) {
    return false;
  }

  vec2 position{base_size_.width() * normal_x, base_size_.height() * normal_y};
  obs_sceneitem_set_pos(item, &position);
  return true;
}


bool Obs::TurnOnChromaKey(const uint32_t &color, const int &similarity) {
  obs_source_t *source = obs_get_source_by_name("Video Capture Device");
  if (source == nullptr) {
    return false;
  }

  obs_source_t *filter =
      obs_source_get_filter_by_name(source, "ChromaKeyFileter");
  if (filter == nullptr) {
    obs_data_t *settings = obs_data_create();
    obs_data_set_string(settings, "key_color_type", "custom");
    obs_data_set_int(settings, "key_color", color);
    obs_data_set_int(settings, "similarity", similarity);
    obs_data_set_int(settings, "smoothness", 80);
    obs_data_set_int(settings, "spill", 100);
    obs_data_set_int(settings, "opacity", 100);
    obs_data_set_double(settings, "contrast", 0.0);
    obs_data_set_double(settings, "brightness", 0.0);
    obs_data_set_double(settings, "gamma", 0.0);

    filter = obs_source_create(
        "chroma_key_filter", "ChromaKeyFileter", settings, nullptr);
    obs_source_filter_add(source, filter);
  } else {
    obs_data_t *settings = obs_source_get_settings(filter);
    obs_data_set_int(settings, "key_color", color);
    obs_data_set_int(settings, "similarity", similarity);
    obs_source_update(filter, settings);
    obs_data_release(settings);
    obs_source_release(filter);
  }
  obs_source_release(filter);
  obs_source_release(source);
  return true;
}


bool Obs::TurnOffChromaKey() {
  obs_source_t *source = obs_get_source_by_name("Video Capture Device");
  if (source == nullptr) {
    return false;
  }
  obs_source_t *filter =
      obs_source_get_filter_by_name(source, "ChromaKeyFileter");
  if (filter == nullptr) {
    return false;
  }
  obs_source_filter_remove(source, filter);
  obs_source_release(source);
  return true;
}


bool Obs::UpdateChromaKeyColor(const uint32_t &color) {
  obs_source_t *source = obs_get_source_by_name("Video Capture Device");
  if (source == nullptr) {
    return false;
  }
  obs_source_t *filter =
      obs_source_get_filter_by_name(source, "ChromaKeyFileter");
  if (filter == nullptr) {
    return false;
  }
  obs_source_release(source);

  obs_data_t *settings = obs_source_get_settings(filter);
  obs_data_set_int(settings, "key_color", color);
  obs_source_update(filter, settings);
  obs_data_release(settings);
  obs_source_release(filter);
  return true;
}


bool Obs::UpdateChromaKeySimilarity(const int &similarity) {
  obs_source_t *source = obs_get_source_by_name("Video Capture Device");
  if (source == nullptr) {
    return false;
  }
  obs_source_t *filter =
      obs_source_get_filter_by_name(source, "ChromaKeyFileter");
  if (filter == nullptr) {
    return false;
  }
  obs_source_release(source);

  obs_data_t *settings = obs_source_get_settings(filter);
  obs_data_set_int(settings, "similarity", similarity);
  obs_source_update(filter, settings);
  obs_data_release(settings);
  obs_source_release(filter);
  return true;
}


void Obs::UpdateVideoQuality(
    const Dimension<uint32_t> &output_size,
    uint32_t fps,
    uint32_t bitrate) {
  output_size_ = output_size;
  fps_ = fps;
  video_bitrate_ = bitrate;
}


void Obs::UpdateCurrentServiceEncoders(
    uint32_t audio_bitrate,
    uint32_t video_bitrate) {
  obs_data_t *video_settings = obs_data_create();
  obs_data_set_string(video_settings, "rate_control", "CBR");
  obs_data_set_int(video_settings, "bitrate", video_bitrate);

  obs_data_t *audio_settings = obs_data_create();
  obs_data_set_string(audio_settings, "rate_control", "CBR");
  obs_data_set_int(audio_settings, "bitrate", audio_bitrate);

  obs_service_apply_encoder_settings(
      current_service_, video_settings, audio_settings);

  video_t *video = obs_get_video();
  enum video_format format = video_output_get_format(video);

  if (format != VIDEO_FORMAT_NV12 && format != VIDEO_FORMAT_I420) {
    obs_encoder_set_preferred_video_format(video_encoder_, VIDEO_FORMAT_NV12);
  }

  obs_encoder_update(video_encoder_, video_settings);
  obs_encoder_update(audio_encoder_, audio_settings);

  obs_data_release(audio_settings);
  obs_data_release(video_settings);
}


void Obs::AddSourceToScene(void *data, obs_scene_t *scene) {
  obs_source_t *source = reinterpret_cast<obs_source_t *>(data);
  obs_sceneitem_t *sceneitem = obs_scene_add(scene, source);
  obs_sceneitem_set_visible(sceneitem, true);
}


Obs::Obs()
    : log_file_{},
      audio_encoder_{nullptr},
      video_encoder_{nullptr},
      stream_output_{},
      scene_{nullptr},
      current_service_{nullptr},
      audio_bitrate_{160},
      video_bitrate_{2500},
      base_size_{1920, 1080},
      output_size_{1280, 720},
      fps_{30} {
  SetUpLog();
  obs_startup("en-US", nullptr, nullptr);
  obs_load_all_modules();
  obs_log_loaded_modules();

  audio_encoder_ = CreateAudioEncoder();
  video_encoder_ = CreateVideoEncoder();

  stream_output_.reset(new ObsOutput{});

  scene_ = obs_scene_create("Scene");

  AddAudioSource();
  ResetAudio();
  ResetVideo();
}


Obs::~Obs() {
  ReleaseCurrentService();
  ClearSceneItems();
  ClearSceneData();

  obs_scene_release(scene_);
  stream_output_.reset();
  obs_encoder_release(video_encoder_);
  obs_encoder_release(audio_encoder_);

  obs_shutdown();
}


bool Obs::SetUpLog() {
  bool dir_created = obs_app::MakeUserDirs();
  if (dir_created == false) {
    return false;
  }

  obs_app::create_log_file(log_file_);
  return true;
}


void Obs::ResetAudio() {
  struct obs_audio_info ai;
  ai.samples_per_sec = 44100;
  ai.speakers = SPEAKERS_STEREO;

  obs_reset_audio(&ai);
}


void Obs::ResetVideo() {
  struct obs_video_info ovi;
  ovi.fps_num = fps_;
  ovi.fps_den = 1;
  ovi.graphics_module = "libobs-d3d11.dll";
  ovi.base_width = base_size_.width();
  ovi.base_height = base_size_.height();
  ovi.output_width = output_size_.width();
  ovi.output_height = output_size_.height();
  ovi.output_format = VIDEO_FORMAT_NV12;
  ovi.colorspace = VIDEO_CS_601;
  ovi.range = VIDEO_RANGE_PARTIAL;
  ovi.adapter = 0;
  ovi.gpu_conversion = true;
  ovi.scale_type = OBS_SCALE_BICUBIC;

  obs_reset_video(&ovi);
}


obs_encoder_t *Obs::CreateAudioEncoder() {
    return obs_audio_encoder_create(
        "ffmpeg_aac", "simple_aac", nullptr, 0, nullptr);
}


obs_encoder_t *Obs::CreateVideoEncoder() {
    return obs_video_encoder_create(
        "obs_x264", "simple_h264_stream", nullptr, nullptr);
}


void Obs::ClearSceneItems() {
  std::vector<obs_sceneitem_t *> items;
  obs_scene_enum_items(scene_,
      [](obs_scene_t *scene, obs_sceneitem_t *item, void *param) {
    std::vector<obs_sceneitem_t *> &items =
        *reinterpret_cast<std::vector<obs_sceneitem_t *> *>(param);
    items.emplace_back(item);
    return true;
  }, &items);

  for (auto &item : items) {
    obs_sceneitem_remove(item);
  }
}


void Obs::ClearSceneData() {
  obs_set_output_source(3, nullptr);
  obs_set_output_source(2, nullptr);
  obs_set_output_source(1, nullptr);
  obs_set_output_source(0, nullptr);
}



void Obs::UpdateVideoSource(const std::string &source_info) {
  obs_data_t *settings = obs_data_create();
  obs_data_set_string(settings, "window", source_info.c_str());
  obs_data_set_string(settings, "capture_mode", "window");
  obs_source_t *source = obs_source_create(
      "game_capture", "Game Capture", settings, nullptr);
  obs_data_release(settings);
  obs_scene_atomic_update(scene_, Obs::AddSourceToScene, source);
  obs_source_release(source);

  obs_set_output_source(0, obs_get_source_by_name("Scene"));
}


void Obs::AddAudioSource() {
  obs_source_t *source = obs_get_output_source(1);
  if (source != nullptr) {
    obs_source_release(source);
    return;
  }

  obs_data_t *settings = obs_data_create();
  obs_data_set_string(settings, "device_id", "default");

  source = obs_source_create(
      "wasapi_output_capture", "Desktop Audio", settings, nullptr);
  obs_data_release(settings);

  obs_set_output_source(1, source);
  obs_source_release(source);
}


void Obs::UpdateCurrentService(
    const std::string &service_provider,
    const std::string &stream_server,
    const std::string &stream_key) {
  ReleaseCurrentService();

  obs_data_t *settings = obs_data_create();
  obs_data_set_string(settings, "service", service_provider.c_str());
  obs_data_set_string(settings, "server", stream_server.c_str());
  obs_data_set_string(settings, "key", stream_key.c_str());
  obs_data_set_bool(settings, "show_all", false);

  current_service_ = obs_service_create(
      "rtmp_common", "default_service", settings, nullptr);
  obs_data_release(settings);
}


void Obs::ReleaseCurrentService() {
  if (!current_service_) {
    return;
  }
  obs_service_release(current_service_);
  current_service_ = nullptr;
}


void Obs::UpdateBaseResolution(const std::string &source_info) {
  ObsSourceInfo source{source_info};
  const std::string &clazz = source.clazz();
  const std::string &title = source.title();

  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  const std::wstring &w_class = converter.from_bytes(clazz);
  const std::wstring &w_title = converter.from_bytes(title);

  HWND handle = ::FindWindowExW(
      nullptr, nullptr, w_class.c_str(), w_title.c_str());
  DWORD process_id;
  GetWindowThreadProcessId(handle, &process_id);
  std::wstring map_name = L"CaptureHook_HookInfo" + std::to_wstring(process_id);
  for (int i = 0; i < 200; ++i) {
    HANDLE hook_info_map = OpenFileMapping(
        FILE_MAP_READ, false, map_name.c_str());
    if (hook_info_map) {
      struct hook_info *info = reinterpret_cast<struct hook_info *>(
          MapViewOfFile(hook_info_map, FILE_MAP_READ, 0, 0, sizeof(info)));
      if (info && info->cx != 0 && info->cy != 0) {
        base_size_ = {info->cx, info->cy};
        break;
      }
    }
    Sleep(100);
  }
}


const bool Obs::CheckDeviceId(const std::string &device_id) {
  const std::vector<std::string> &webcams{FindAllWebcamDevices()};
  for (auto webcam : webcams) {
    if (webcam == device_id) {
      return true;
    }
  }
  return false;
}


const std::string Obs::DecodeObsString(
    const std::string &encoded_string) {
  std::string decoded{encoded_string};
  ReplaceString(&decoded, "#3A", ":");
  ReplaceString(&decoded, "#22", "#");
  return decoded;
}


void Obs::ReplaceString(
      std::string *subject,
      const std::string &search,
      const std::string &replace) {
  size_t pos{0};
  while ((pos = subject->find(search, pos)) != std::string::npos) {
    subject->replace(pos, search.length(), replace);
    pos += replace.length();
  }
}


Obs *Obs::static_instance{nullptr};
}  // namespace ncstreamer
