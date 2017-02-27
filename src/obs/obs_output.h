/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_OBS_OBS_OUTPUT_H_
#define SRC_OBS_OBS_OUTPUT_H_


#include "obs-studio/libobs/obs.h"


namespace ncstreamer {
class ObsOutput {
 public:
  ObsOutput();
  virtual ~ObsOutput();

  bool Start(obs_encoder_t *audio_encoder,
             obs_encoder_t *video_encoder,
             obs_service_t *service);
  void Stop();

 private:
  obs_output_t *output_;
};
}  // namespace ncstreamer


#endif  // SRC_OBS_OBS_OUTPUT_H_
