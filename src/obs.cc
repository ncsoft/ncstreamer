/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/obs.h"

#include <cassert>

#include "src_imported/from_obs_studio_ui/obs-app.hpp"


namespace ncstreamer {
void Obs::SetUp() {
  assert(!static_instance);
  static_instance = new Obs{};
}


Obs::Obs()
    : log_file_{},
      audio_encoder_{nullptr},
      video_encoder_{nullptr} {
  SetUpLog();
  obs_startup("en-US", nullptr, nullptr);
  obs_load_all_modules();
  obs_log_loaded_modules();

  ResetAudio();
  ResetVideo();

  audio_encoder_ = CreateAudioEncoder();
  video_encoder_ = CreateVideoEncoder();
  obs_encoder_set_audio(audio_encoder_,  obs_get_audio());
  obs_encoder_set_video(video_encoder_, obs_get_video());
}


void Obs::ShutDown() {
  assert(static_instance);
  delete static_instance;
  static_instance = nullptr;
}


Obs::~Obs() {
  obs_encoder_release(video_encoder_);
  obs_encoder_release(audio_encoder_);

  obs_shutdown();
}


Obs *Obs::Get() {
  assert(static_instance);
  return static_instance;
}


std::vector<std::string> Obs::FindAllWindowsOnDesktop() {
  std::vector<std::string> titles;

  obs_source_t *source =
      obs_source_create("window_capture", "Window Capture", nullptr, nullptr);
  obs_properties_t *props = obs_source_properties(source);
  obs_property_t *prop = obs_properties_get(props, "window");

  int count = obs_property_list_item_count(prop);
  for (int i = 0; i < count; i++) {
    const char *val = obs_property_list_item_string(prop, i);
    titles.emplace_back(val);
  }

  obs_properties_destroy(props);
  obs_source_release(source);
  return titles;
}


bool Obs::StartStreaming(
    const std::string &source_info,
    const std::string &service_provider,
    const std::string &stream_url) {
  // TODO(khpark): TBD
  return false;
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
  ovi.fps_num = 30;
  ovi.fps_den = 1;
  ovi.graphics_module = "libobs-d3d11.dll";
  ovi.base_width = 1920;
  ovi.base_height = 1080;
  ovi.output_width = 1280;
  ovi.output_height = 720;
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


Obs *Obs::static_instance{nullptr};
}  // namespace ncstreamer
