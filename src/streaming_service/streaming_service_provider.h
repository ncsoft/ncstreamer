/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_STREAMING_SERVICE_STREAMING_SERVICE_PROVIDER_H_
#define SRC_STREAMING_SERVICE_STREAMING_SERVICE_PROVIDER_H_


#include <functional>
#include <string>
#include <vector>

#include "windows.h"  // NOLINT


namespace ncstreamer {
class StreamingServiceProvider {
 public:
  using OnFailed =
      std::function<void(const std::wstring &fail)>;
  using OnLoggedIn =
      std::function<void(const std::wstring &user_name,
                         const std::vector<std::wstring> &user_pages)>;

  StreamingServiceProvider();
  virtual ~StreamingServiceProvider() = 0;

  virtual void LogIn(
      HWND parent,
      const OnFailed &on_failed,
      const OnLoggedIn &on_logged_in) = 0;
};
}  // namespace ncstreamer


#endif  // SRC_STREAMING_SERVICE_STREAMING_SERVICE_PROVIDER_H_
