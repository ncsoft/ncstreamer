/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_OBS_OBS_OUTPUT_H_
#define SRC_OBS_OBS_OUTPUT_H_


#include <functional>
#include <memory>

#include "obs-studio/libobs/obs.h"


namespace ncstreamer {
class ObsOutput {
 public:
  using OnStarted = std::function<void()>;
  using OnStopped = std::function<void()>;

  ObsOutput();
  virtual ~ObsOutput();

  bool Start(obs_encoder_t *audio_encoder,
             obs_encoder_t *video_encoder,
             obs_service_t *service,
             const OnStarted &on_started);
  void Stop(const OnStopped &on_stopped);

 private:
  static void OnStartSignal(void *data, calldata_t *params);
  static void OnStopSignal(void *data, calldata_t *params);

  obs_output_t *output_;
  signal_handler_t *const signal_handler_;

  std::unique_ptr<OnStarted> on_started_;
  std::unique_ptr<OnStopped> on_stopped_;
};
}  // namespace ncstreamer


#endif  // SRC_OBS_OBS_OUTPUT_H_
