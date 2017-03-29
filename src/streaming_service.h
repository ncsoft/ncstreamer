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

  static void SetUp();
  static void ShutDown();
  static StreamingService *Get();

  void LogIn(
      const std::wstring &service_provider_id,
      HWND parent,
      const OnFailed &on_failed,
      const OnLoggedIn &on_logged_in);

 private:
  class FailMessage {
   public:
    static std::wstring ToUnknownServiceProvider(
        const std::wstring &service_provider_id);
  };

  StreamingService();
  virtual ~StreamingService();

  static StreamingService *static_instance;

  std::unordered_map<
      std::wstring /*provider_id*/,
      std::shared_ptr<StreamingServiceProvider>> service_providers_;

  std::shared_ptr<StreamingServiceProvider> current_service_provider_;
};
}  // namespace ncstreamer


#endif  // SRC_STREAMING_SERVICE_H_
