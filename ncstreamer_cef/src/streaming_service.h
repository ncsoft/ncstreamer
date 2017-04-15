/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_STREAMING_SERVICE_H_
#define NCSTREAMER_CEF_SRC_STREAMING_SERVICE_H_


#include <memory>
#include <string>
#include <unordered_map>

#include "windows.h"  // NOLINT

#include "ncstreamer_cef/src/streaming_service/streaming_service_provider.h"


namespace ncstreamer {
class StreamingService {
 public:
  using OnFailed = StreamingServiceProvider::OnFailed;
  using OnLoggedIn = StreamingServiceProvider::OnLoggedIn;
  using OnLiveVideoPosted =
      std::function<void(const std::string &service_provider,
                         const std::string &stream_url)>;

  static void SetUp();
  static void ShutDown();
  static StreamingService *Get();

  void LogIn(
      const std::string &service_provider_id,
      HWND parent,
      const std::wstring &locale,
      const OnFailed &on_failed,
      const OnLoggedIn &on_logged_in);

  void PostLiveVideo(
      const std::string &user_page_id,
      const std::string &privacy,
      const std::string &title,
      const std::string &description,
      const OnFailed &on_failed,
      const OnLiveVideoPosted &on_live_video_posted);

 private:
  class FailMessage {
   public:
    static std::string ToUnknownServiceProvider(
        const std::string &service_provider_id);
    static std::string ToNotLoggedIn();
  };

  StreamingService();
  virtual ~StreamingService();

  static StreamingService *static_instance;

  std::unordered_map<
      std::string /*service_provider_id*/,
      std::shared_ptr<StreamingServiceProvider>> service_providers_;

  const std::string *current_service_provider_id_;
  std::shared_ptr<StreamingServiceProvider> current_service_provider_;
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_STREAMING_SERVICE_H_
