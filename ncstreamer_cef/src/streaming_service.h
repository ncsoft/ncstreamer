/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_STREAMING_SERVICE_H_
#define NCSTREAMER_CEF_SRC_STREAMING_SERVICE_H_


#include <memory>
#include <string>
#include <unordered_map>

#include "boost/optional.hpp"
#include "windows.h"  // NOLINT

#include "ncstreamer_cef/src/streaming_service/streaming_service_provider.h"
#include "ncstreamer_cef/src/streaming_service/streaming_service_types.h"


namespace ncstreamer {
class StreamingService {
 public:
  using OnFailed = StreamingServiceProvider::OnFailed;
  using OnLoggedIn = StreamingServiceProvider::OnLoggedIn;
  using OnLoggedOut = StreamingServiceProvider::OnLoggedOut;
  using OnLiveVideoPosted =
      std::function<void(const std::string &service_provider,
                         const std::string &stream_server,
                         const std::string &stream_key,
                         const std::string &video_id,
                         const std::string &post_url)>;
  using OnCommentsGot =
      std::function<void(const std::string &comments)>;
  using OnLiveVideoViewers =
      std::function<void(const std::string &viewers)>;

  static void SetUp(
      const StreamingServiceTagMap &tag_ids);

  static void ShutDown();
  static StreamingService *Get();

  void LogIn(
      const std::string &service_provider_id,
      HWND parent,
      const std::wstring &locale,
      const OnFailed &on_failed,
      const OnLoggedIn &on_logged_in);

  void LogOut(
      const std::string &service_provider_id,
      const OnFailed &on_failed,
      const OnLoggedOut &on_logged_out);

  void PostLiveVideo(
      const std::string &stream_server,
      const std::string &user_page_id,
      const std::string &privacy,
      const std::string &title,
      const std::string &description,
      const std::string &source,
      const OnFailed &on_failed,
      const OnLiveVideoPosted &on_live_video_posted);

  void GetComments(
      const std::string &created_time,
      const OnFailed &on_failed,
      const OnCommentsGot &on_comments_got);

  void GetLiveVideoViewers(
      const OnFailed &on_failed,
      const OnLiveVideoViewers &on_live_video_viewers);

  void StopLiveVideo();

  void LogOutAll();

 private:
  class FailMessage {
   public:
    static std::string ToUnknownServiceProvider(
        const std::string &service_provider_id);
    static std::string ToNotLoggedIn();
  };

  explicit StreamingService(
      const StreamingServiceTagMap &tag_ids);

  virtual ~StreamingService();

  const std::string &FindTagId(
      const std::string &service_provider,
      const std::string &source) const;

  void HandleFail(
      const OnFailed &on_failed,
      const std::string &func,
      const std::string &msg);

  static StreamingService *static_instance;

  const StreamingServiceTagMap tag_ids_;

  std::unordered_map<
      std::string /*service_provider_id*/,
      std::shared_ptr<StreamingServiceProvider>> service_providers_;

  const std::string *current_service_provider_id_;
  std::shared_ptr<StreamingServiceProvider> current_service_provider_;
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_STREAMING_SERVICE_H_
