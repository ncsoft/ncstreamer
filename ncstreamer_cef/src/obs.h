/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_OBS_H_
#define NCSTREAMER_CEF_SRC_OBS_H_


#include <fstream>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "boost/property_tree/ptree.hpp"
#include "obs-studio/libobs/obs.h"
#include "obs-studio/plugins/win-dshow/libdshowcapture/dshowcapture.hpp"

#include "ncstreamer_cef/src/lib/dimension.h"
#include "ncstreamer_cef/src/obs/obs_output.h"


namespace ncstreamer {
class Obs {
 public:
  class WebcamDevice;

  static void SetUp();
  static void ShutDown();
  static Obs *Get();

  std::vector<std::string> FindAllWindowsOnDesktop();

  bool StartStreaming(
      const std::string &source_info,
      const std::string &service_provider,
      const std::string &stream_server,
      const std::string &stream_key,
      const ObsOutput::OnStarted &on_streaming_started);
  void StopStreaming(
      const ObsOutput::OnStopped &on_streaming_stopped);

  bool SearchMicDevices();
  bool TurnOnMic(std::string *error);
  bool TurnOffMic();
  bool UpdateMicVolume(float volume);
  std::vector<Obs::WebcamDevice> SearchWebcamDevices();
  bool TurnOnWebcam(const std::string &device_id);
  bool TurnOffWebcam();
  bool UpdateWebcamSize(const float normal_x, const float normal_y);
  bool UpdateWebcamPosition(const float normal_x, const float normal_y);
  bool TurnOnChromaKey(const uint32_t color, const int similarity);
  bool TurnOffChromaKey();
  bool UpdateChromaKeyColor(const uint32_t color);
  bool UpdateChromaKeySimilarity(const int similarity);
  void UpdateVideoQuality(
      const Dimension<uint32_t> &output_size,
      uint32_t fps,
      uint32_t bitrate);

  void UpdateCurrentServiceEncoders(
      uint32_t audio_bitrate,
      uint32_t video_bitrate);

 private:
  static void AddSourceToScene(void *data, obs_scene_t *scene);

  Obs();
  virtual ~Obs();

  bool SetUpLog();
  void ResetAudio();
  void ResetVideo();
  obs_encoder_t *CreateAudioEncoder();
  obs_encoder_t *CreateVideoEncoder();
  void ClearSceneItems();
  void ClearSceneData();

  void UpdateCurrentSource(const std::string &source_info);

  void UpdateCurrentService(
      const std::string &service_provider,
      const std::string &stream_server,
      const std::string &stream_key);
  void ReleaseCurrentService();
  void UpdateBaseResolution(const std::string &source_info);
  bool GetDevice(std::string device_id, DShow::VideoDevice *device);

  static Obs *static_instance;

  std::fstream log_file_;
  obs_encoder_t *audio_encoder_;
  obs_encoder_t *video_encoder_;
  std::unique_ptr<ObsOutput> stream_output_;

  obs_service_t *current_service_;
  int audio_bitrate_;
  int video_bitrate_;
  Dimension<uint32_t> base_size_;
  Dimension<uint32_t> output_size_;
  uint32_t fps_;
  std::vector<DShow::VideoDevice> video_devices_;
  std::vector<DShow::AudioDevice> audio_devices_;
};


class Obs::WebcamDevice {
 public:
  WebcamDevice(const std::string &device_id,
               const Dimension<uint32_t> &default_size);
  virtual ~WebcamDevice();

  boost::property_tree::ptree ToTree() const;

 private:
  std::string device_id_;
  Dimension<uint32_t> default_size_;
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_OBS_H_
