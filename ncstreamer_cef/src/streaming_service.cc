/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/streaming_service.h"

#include <cassert>

#include "ncstreamer_cef/src/streaming_service/facebook.h"
#include "ncstreamer_cef/src/streaming_service/twitch.h"


namespace ncstreamer {
void StreamingService::SetUp(
    const StreamingServiceTagMap &tag_ids) {
  assert(!static_instance);
  static_instance = new StreamingService{tag_ids};
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


StreamingService::StreamingService(
    const StreamingServiceTagMap &tag_ids)
    : tag_ids_{tag_ids},
      service_providers_{
          {"Facebook Live", std::shared_ptr<Facebook>{new Facebook{}}},
          {"Twitch", std::shared_ptr<Twitch>{new Twitch{}}}},
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
  static const std::string kFunc{"LogIn"};

  auto i = service_providers_.find(service_provider_id);
  if (i == service_providers_.end()) {
    HandleFail(on_failed, kFunc,
        FailMessage::ToUnknownServiceProvider(service_provider_id));
    return;
  }
  current_service_provider_id_ = &(i->first);
  current_service_provider_ = i->second;

  current_service_provider_->LogIn(
      parent,
      locale,
      [this, on_failed](const std::string &error) {
        HandleFail(on_failed, kFunc, error);
      },
      on_logged_in);
}


void StreamingService::LogOut(
    const std::string &service_provider_id,
    const OnFailed &on_failed,
    const OnLoggedOut &on_logged_out) {
  static const std::string kFunc{"LogOut"};

  auto i = service_providers_.find(service_provider_id);
  if (i == service_providers_.end()) {
    HandleFail(on_failed, kFunc,
        FailMessage::ToUnknownServiceProvider(service_provider_id));
    return;
  }
  current_service_provider_id_ = &(i->first);
  current_service_provider_ = i->second;

  current_service_provider_->LogOut(
      [this, on_failed](const std::string &error) {
        HandleFail(on_failed, kFunc, error);
      },
      on_logged_out);
}


void StreamingService::PostLiveVideo(
    const std::string &user_page_id,
    const std::string &privacy,
    const std::string &title,
    const std::string &description,
    const std::string &source,
    const OnFailed &on_failed,
    const OnLiveVideoPosted &on_live_video_posted) {
  static const std::string kFunc{"PostLiveVideo"};

  if (!current_service_provider_) {
    HandleFail(on_failed, kFunc,
        FailMessage::ToNotLoggedIn());
    return;
  }

  const std::string &service_provider_id = *current_service_provider_id_;
  const std::string &tag_id = FindTagId(service_provider_id, source);

  current_service_provider_->PostLiveVideo(
      user_page_id,
      privacy,
      title,
      description,
      tag_id,
      [this, on_failed](const std::string &error) {
        HandleFail(on_failed, kFunc, error);
      },
      [on_live_video_posted, service_provider_id](
          const std::string &stream_server,
          const std::string &stream_key,
          const std::string &post_url) {
        on_live_video_posted(
            service_provider_id, stream_server, stream_key, post_url);
      });
}


void StreamingService::LogOutAll() {
  for (const auto &elem : service_providers_) {
    auto service_provider = elem.second;
    service_provider->LogOut([](
        const std::string &/*fail*/) {
    }, []() {
    });
  }
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


const std::string &StreamingService::FindTagId(
    const std::string &service_provider,
    const std::string &source) const {
  static const std::string kEmptyTagId{""};

  auto source_tags_i = tag_ids_.find(service_provider);
  if (source_tags_i == tag_ids_.end()) {
    return kEmptyTagId;
  }
  const auto &source_tags = source_tags_i->second;
  auto tag_i = source_tags.find(source);
  if (tag_i == source_tags.end()) {
    return kEmptyTagId;
  }
  return tag_i->second;
}


void StreamingService::HandleFail(
    const OnFailed &on_failed,
    const std::string &func,
    const std::string &msg) {
  std::stringstream ss;
  ss << "StreamingService";
  if (current_service_provider_id_) {
    ss << "[" << *current_service_provider_id_ << "]";
  }
  ss << ": " << func
     << ": " << msg;
  on_failed(ss.str());
}


StreamingService *StreamingService::static_instance{nullptr};
}  // namespace ncstreamer
