/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/obs/obs_output.h"


namespace ncstreamer {
ObsOutput::ObsOutput()
    : output_{obs_output_create(
          "rtmp_output", "simple_stream", nullptr, nullptr)},
      signal_handler_{obs_output_get_signal_handler(output_)},
      on_started_{},
      on_stopped_{} {
  obs_data_t *settings = obs_data_create();
  obs_data_set_string(settings, "bind_ip", "default");
  obs_output_update(output_, settings);
  obs_data_release(settings);

  obs_output_set_delay(output_, 0, OBS_OUTPUT_DELAY_PRESERVE);
  obs_output_set_reconnect_settings(
      output_, /*max_retries*/ 20, /*retry_delay*/ 10);
}


ObsOutput::~ObsOutput() {
  on_stopped_.reset();
  on_started_.reset();

  obs_output_release(output_);
  output_ = nullptr;
}


bool ObsOutput::Start(obs_encoder_t *audio_encoder,
                      obs_encoder_t *video_encoder,
                      obs_service_t *service,
                      const OnStarted &on_started) {
  obs_output_set_audio_encoder(output_, audio_encoder, 0);
  obs_output_set_video_encoder(output_, video_encoder);
  obs_output_set_service(output_, service);

  signal_handler_disconnect(
      signal_handler_, "start", OnStartSignal, on_started_.get());
  on_started_.reset(new OnStarted{on_started});
  signal_handler_connect(
      signal_handler_, "start", OnStartSignal, on_started_.get());

  return obs_output_start(output_);
}


void ObsOutput::Stop(const OnStopped &on_stopped) {
  signal_handler_disconnect(
      signal_handler_, "stop", OnStopSignal, on_stopped_.get());
  on_stopped_.reset(new OnStopped{on_stopped});
  signal_handler_connect(
      signal_handler_, "stop", OnStopSignal, on_stopped_.get());

  obs_output_stop(output_);
}


void ObsOutput::OnStartSignal(void *data, calldata_t * /*params*/) {
  auto on_started = reinterpret_cast<OnStarted *>(data);
  (*on_started)();
}


void ObsOutput::OnStopSignal(void *data, calldata_t * /*params*/) {
  auto on_stopped = reinterpret_cast<OnStarted *>(data);
  (*on_stopped)();
}
}  // namespace ncstreamer
