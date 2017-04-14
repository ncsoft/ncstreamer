/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/streaming_service.h"

#include <cassert>

#include "ncstreamer_cef/src/streaming_service/facebook.h"


namespace ncstreamer {
void StreamingService::SetUp() {
  assert(!static_instance);
  static_instance = new StreamingService{};
}


void StreamingService::ShutDown() {
  assert(static_instance);
  delete static_instance;
  static_instance = nullptr;
}


StreamingService *StreamingService::Get() {
  assert(static_instance);
  return static_instance;
}


StreamingService::StreamingService()
    : service_providers_{
          {"Facebook Live", std::shared_ptr<Facebook>{new Facebook{}}}},
      current_service_provider_id_{nullptr},
      current_service_provider_{} {
}


StreamingService::~StreamingService() {
}


void StreamingService::LogIn(
    const std::string &service_provider_id,
    HWND parent,
    const std::wstring &locale,
    const OnFailed &on_failed,
    const OnLoggedIn &on_logged_in) {
  auto i = service_providers_.find(service_provider_id);
  if (i == service_providers_.end()) {
    on_failed(FailMessage::ToUnknownServiceProvider(service_provider_id));
    return;
  }
  current_service_provider_id_ = &(i->first);
  current_service_provider_ = i->second;

  current_service_provider_->LogIn(
      parent,
      locale,
      on_failed,
      on_logged_in);
}


void StreamingService::PostLiveVideo(
    const std::string &user_page_id,
    const std::string &privacy,
    const std::string &title,
    const std::string &description,
    const OnFailed &on_failed,
    const OnLiveVideoPosted &on_live_video_posted) {
  if (!current_service_provider_) {
    on_failed(FailMessage::ToNotLoggedIn());
    return;
  }

  const std::string &service_provider_id = *current_service_provider_id_;
  current_service_provider_->PostLiveVideo(
      user_page_id,
      privacy,
      title,
      description,
      on_failed,
      [on_live_video_posted, service_provider_id](
          const std::string &stream_url) {
        on_live_video_posted(service_provider_id, stream_url);
      });
}


std::string StreamingService::FailMessage::ToUnknownServiceProvider(
    const std::string &service_provider_id) {
  std::stringstream msg;
  msg << "unknown service provider: " << service_provider_id;
  return msg.str();
}


std::string StreamingService::FailMessage::ToNotLoggedIn() {
  std::stringstream msg;
  msg << "not logged in";
  return msg.str();
}


StreamingService *StreamingService::static_instance{nullptr};
}  // namespace ncstreamer
