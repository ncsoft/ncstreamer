/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/streaming_service.h"

#include <cassert>

#include "src/streaming_service/facebook.h"


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
          {L"Facebook Live", std::shared_ptr<Facebook>{new Facebook{}}}} {
}


StreamingService::~StreamingService() {
}


void StreamingService::LogIn(
    const std::wstring &service_provider_id,
    HWND parent,
    const StreamingServiceProvider::OnFailed &on_failed,
    const StreamingServiceProvider::OnLoggedIn &on_logged_in) {
  auto i = service_providers_.find(service_provider_id);
  if (i == service_providers_.end()) {
    on_failed(FailMessage::ToUnknownServiceProvider(service_provider_id));
    return;
  }
  const auto &service_provider = i->second;

  service_provider->LogIn(
      parent,
      on_failed,
      on_logged_in);
}


std::wstring StreamingService::FailMessage::ToUnknownServiceProvider(
    const std::wstring &service_provider_id) {
  std::wstringstream msg;
  msg << L"unknown service provider: " << service_provider_id;
  return msg.str();
}


StreamingService *StreamingService::static_instance{nullptr};
}  // namespace ncstreamer
