/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_OBS_H_
#define SRC_OBS_H_


#include <fstream>
#include <string>
#include <vector>

#include "obs-studio/libobs/obs.h"


namespace ncstreamer {
class Obs {
 public:
  static void SetUp();
  static void ShutDown();
  static Obs *Get();

  std::vector<std::string> FindAllWindowsOnDesktop();

 private:
  Obs();
  virtual ~Obs();

  bool SetUpLog();
  void ResetAudio();
  void ResetVideo();
  obs_encoder_t *CreateAudioEncoder();
  obs_encoder_t *CreateVideoEncoder();

  static Obs *static_instance;

  std::fstream log_file_;
  obs_encoder_t *audio_encoder_;
  obs_encoder_t *video_encoder_;
};
}  // namespace ncstreamer


#endif  // SRC_OBS_H_
