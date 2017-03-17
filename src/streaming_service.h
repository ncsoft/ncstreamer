/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_STREAMING_SERVICE_H_
#define SRC_STREAMING_SERVICE_H_


#include <memory>
#include <unordered_map>

#include "src/streaming_service/streaming_service_provider.h"


namespace ncstreamer {
class StreamingService {
 public:
  static void SetUp();
  static void ShutDown();
  static StreamingService *Get();

 private:
  StreamingService();
  virtual ~StreamingService();

  static StreamingService *static_instance;

  std::unordered_map<
      std::wstring /*provider_id*/,
      std::shared_ptr<StreamingServiceProvider>> service_providers_;
};
}  // namespace ncstreamer


#endif  // SRC_STREAMING_SERVICE_H_
