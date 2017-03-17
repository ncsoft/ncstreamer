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


StreamingService *StreamingService::static_instance{nullptr};
}  // namespace ncstreamer
