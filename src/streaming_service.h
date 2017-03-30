/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_STREAMING_SERVICE_H_
#define SRC_STREAMING_SERVICE_H_


#include <memory>
#include <unordered_map>

#include "windows.h"  // NOLINT

#include "src/streaming_service/streaming_service_provider.h"


namespace ncstreamer {
class StreamingService {
 public:
  using OnFailed = StreamingServiceProvider::OnFailed;
  using OnLoggedIn = StreamingServiceProvider::OnLoggedIn;
  using OnLiveVideoPosted =
      std::function<void(const std::wstring &service_provider,
                         const std::wstring &stream_url)>;

  static void SetUp();
  static void ShutDown();
  static StreamingService *Get();

  void LogIn(
      const std::wstring &service_provider_id,
      HWND parent,
      const OnFailed &on_failed,
      const OnLoggedIn &on_logged_in);

  void PostLiveVideo(
      const std::wstring &user_page_id,
      const std::wstring &privacy,
      const std::wstring &title,
      const std::wstring &description,
      const OnFailed &on_failed,
      const OnLiveVideoPosted &on_live_video_posted);

 private:
  class FailMessage {
   public:
    static std::wstring ToUnknownServiceProvider(
        const std::wstring &service_provider_id);
    static std::wstring ToNotLoggedIn();
  };

  StreamingService();
  virtual ~StreamingService();

  static StreamingService *static_instance;

  std::unordered_map<
      std::wstring /*service_provider_id*/,
      std::shared_ptr<StreamingServiceProvider>> service_providers_;

  const std::wstring *current_service_provider_id_;
  std::shared_ptr<StreamingServiceProvider> current_service_provider_;
};
}  // namespace ncstreamer


#endif  // SRC_STREAMING_SERVICE_H_
