/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_OBS_H_
#define SRC_OBS_H_


#include <fstream>
#include <string>
#include <tuple>
#include <vector>

#include "obs-studio/libobs/obs.h"


namespace ncstreamer {
class Obs {
 public:
  static void SetUp();
  static void ShutDown();
  static Obs *Get();

  std::vector<std::string> FindAllWindowsOnDesktop();

  bool StartStreaming(
      const std::string &source_info,
      const std::string &service_provider,
      const std::string &stream_url);

  void UpdateCurrentServiceEncoders(
      uint32_t audio_bitrate,
      uint32_t video_bitrate);

 private:
  Obs();
  virtual ~Obs();

  static std::tuple<std::string /*server*/, std::string /*key*/>
      Obs::SplitStreamUrl(const std::string &stream_url);

  bool SetUpLog();
  void ResetAudio();
  void ResetVideo();
  obs_encoder_t *CreateAudioEncoder();
  obs_encoder_t *CreateVideoEncoder();
  obs_output_t *CreateOutput();

  void UpdateCurrentSource(const std::string &source_info);
  void ReleaseCurrentSource();

  void UpdateCurrentService(
      const std::string &service_provider,
      const std::string &stream_server,
      const std::string &stream_key);
  void ReleaseCurrentService();

  static Obs *static_instance;

  std::fstream log_file_;
  obs_encoder_t *audio_encoder_;
  obs_encoder_t *video_encoder_;
  obs_output_t *stream_output_;

  obs_source_t *current_source_video_;
  obs_service_t *current_service_;
};
}  // namespace ncstreamer


#endif  // SRC_OBS_H_
