/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/obs/obs_output.h"


namespace ncstreamer {
ObsOutput::ObsOutput()
    : output_{obs_output_create(
          "rtmp_output", "simple_stream", nullptr, nullptr)} {
  obs_data_t *settings = obs_data_create();
  obs_data_set_string(settings, "bind_ip", "default");
  obs_output_update(output_, settings);
  obs_data_release(settings);

  obs_output_set_delay(output_, 0, OBS_OUTPUT_DELAY_PRESERVE);
  obs_output_set_reconnect_settings(
      output_, /*max_retries*/ 20, /*retry_delay*/ 10);
}


ObsOutput::~ObsOutput() {
  obs_output_release(output_);
  output_ = nullptr;
}


bool ObsOutput::Start(obs_encoder_t *audio_encoder,
                      obs_encoder_t *video_encoder,
                      obs_service_t *service) {
  obs_output_set_audio_encoder(output_, audio_encoder, 0);
  obs_output_set_video_encoder(output_, video_encoder);
  obs_output_set_service(output_, service);

  return obs_output_start(output_);
}


void ObsOutput::Stop() {
  obs_output_stop(output_);
}
}  // namespace ncstreamer
