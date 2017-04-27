/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/obs.h"

#include <cassert>

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
    const std::string &stream_url,
    const ObsOutput::OnStarted &on_streaming_started) {
  UpdateCurrentSource(source_info);

  std::string stream_server;
  std::string stream_key;
  std::tie(stream_server, stream_key) = SplitStreamUrl(stream_url);
  UpdateCurrentService(service_provider, stream_server, stream_key);
  UpdateCurrentServiceEncoders(
      /*audio_bitrate*/ 160,
      /*video_bitrate*/ 2500);

  return stream_output_->Start(
      audio_encoder_, video_encoder_, current_service_, on_streaming_started);
}


void Obs::StopStreaming(
    const ObsOutput::OnStopped &on_streaming_stopped) {
  stream_output_->Stop(on_streaming_stopped);
}


void Obs::TurnOnMic() {
  obs_data_t *settings = obs_data_create();
  obs_data_set_string(settings, "device_id", "default");

  obs_source_t *source = obs_source_create(
      "wasapi_input_capture", "Mic/Aux", settings, nullptr);
  obs_data_release(settings);

  obs_set_output_source(3, source);
  obs_source_release(source);
}


void Obs::TurnOffMic() {
  obs_set_output_source(3, nullptr);
}


void Obs::UpdateVideoQuality(
    const Dimension<uint32_t> &output_size,
    uint32_t fps,
    uint32_t bitrate) {
  ResetVideo(output_size, fps);
  obs_encoder_set_video(video_encoder_, obs_get_video());
  UpdateCurrentServiceEncoders(160, bitrate);
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


Obs::Obs()
    : log_file_{},
      audio_encoder_{nullptr},
      video_encoder_{nullptr},
      stream_output_{},
      current_service_{nullptr} {
  SetUpLog();
  obs_startup("en-US", nullptr, nullptr);
  obs_load_all_modules();
  obs_log_loaded_modules();

  ResetAudio();
  ResetVideo({1280, 720}, 30);

  audio_encoder_ = CreateAudioEncoder();
  video_encoder_ = CreateVideoEncoder();
  obs_encoder_set_audio(audio_encoder_,  obs_get_audio());
  obs_encoder_set_video(video_encoder_, obs_get_video());

  stream_output_.reset(new ObsOutput{});
}


Obs::~Obs() {
  ReleaseCurrentService();
  ClearSceneData();

  stream_output_.reset();
  obs_encoder_release(video_encoder_);
  obs_encoder_release(audio_encoder_);

  obs_shutdown();
}


std::tuple<std::string /*server*/, std::string /*key*/>
    Obs::SplitStreamUrl(const std::string &stream_url) {
  std::size_t key_index = stream_url.find_last_of('/') + 1;
  return std::make_tuple(stream_url.substr(0, key_index),
                         stream_url.substr(key_index));
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


void Obs::ResetVideo(const Dimension<uint32_t> &output_size, uint32_t fps) {
  struct obs_video_info ovi;
  ovi.fps_num = fps;
  ovi.fps_den = 1;
  ovi.graphics_module = "libobs-d3d11.dll";
  ovi.base_width = 1920;
  ovi.base_height = 1080;
  ovi.output_width = output_size.width();
  ovi.output_height = output_size.height();
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


void Obs::ClearSceneData() {
  obs_set_output_source(3, nullptr);
  obs_set_output_source(1, nullptr);
  obs_set_output_source(0, nullptr);
}


void Obs::UpdateCurrentSource(const std::string &source_info) {
  // video
  {
    obs_data_t *settings = obs_data_create();
    obs_data_set_string(settings, "window", source_info.c_str());

    obs_source_t *source = obs_source_create(
        "window_capture", "Window Capture", settings, nullptr);
    obs_data_release(settings);

    obs_set_output_source(0, source);
    obs_source_release(source);
  }

  // audio
  {
    obs_data_t *settings = obs_data_create();
    obs_data_set_string(settings, "device_id", "default");

    obs_source_t *source = obs_source_create(
        "wasapi_output_capture", "Desktop Audio", settings, nullptr);
    obs_data_release(settings);

    obs_set_output_source(1, source);
    obs_source_release(source);
  }
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


Obs *Obs::static_instance{nullptr};
}  // namespace ncstreamer
