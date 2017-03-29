/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_STREAMING_SERVICE_STREAMING_SERVICE_PROVIDER_H_
#define SRC_STREAMING_SERVICE_STREAMING_SERVICE_PROVIDER_H_


#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "boost/property_tree/ptree.hpp"
#include "windows.h"  // NOLINT


namespace ncstreamer {
class StreamingServiceProvider {
 public:
  class UserPage;

  using OnFailed =
      std::function<void(const std::wstring &fail)>;
  using OnLoggedIn =
      std::function<void(const std::wstring &user_name,
                         const std::vector<UserPage> &user_pages)>;
  using OnLiveVideoPosted =
      std::function<void(const std::wstring &stream_url)>;

  StreamingServiceProvider();
  virtual ~StreamingServiceProvider() = 0;

  virtual void LogIn(
      HWND parent,
      const OnFailed &on_failed,
      const OnLoggedIn &on_logged_in) = 0;

  virtual void PostLiveVideo(
      const std::wstring &user_page_id,
      const std::wstring &description,
      const OnFailed &on_failed,
      const OnLiveVideoPosted &on_live_video_posted) = 0;
};


class StreamingServiceProvider::UserPage {
 public:
  UserPage(
      const std::wstring &id,
      const std::wstring &name);
  virtual ~UserPage();

  const std::wstring &id() const { return id_; }
  const std::wstring &name() const { return name_; }

  boost::property_tree::ptree ToTree() const;

 private:
  const std::wstring id_;
  const std::wstring name_;
};
}  // namespace ncstreamer


#endif  // SRC_STREAMING_SERVICE_STREAMING_SERVICE_PROVIDER_H_
