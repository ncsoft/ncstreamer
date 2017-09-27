/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_STREAMING_SERVICE_STREAMING_SERVICE_PROVIDER_H_
#define NCSTREAMER_CEF_SRC_STREAMING_SERVICE_STREAMING_SERVICE_PROVIDER_H_


#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "boost/property_tree/ptree.hpp"
#include "windows.h"  // NOLINT


namespace ncstreamer {
class StreamingServiceProvider {
 public:
  class StreamServer;
  class UserPage;

  using OnFailed =
      std::function<void(const std::string &fail)>;
  using OnLoggedIn =
      std::function<void(const std::string &user_name,
                         const std::vector<UserPage> &user_pages,
                         const std::vector<StreamServer> &stream_servers)>;
  using OnLoggedOut =
      std::function<void()>;
  using OnLiveVideoPosted =
      std::function<void(const std::string &stream_server,
                         const std::string &stream_key,
                         const std::string &post_url)>;

  StreamingServiceProvider();
  virtual ~StreamingServiceProvider() = 0;

  virtual void LogIn(
      HWND parent,
      const std::wstring &locale,
      const OnFailed &on_failed,
      const OnLoggedIn &on_logged_in) = 0;

  virtual void LogOut(
      const OnFailed &on_failed,
      const OnLoggedOut &on_logged_out) = 0;

  virtual void PostLiveVideo(
      const std::string &stream_server,
      const std::string &user_page_id,
      const std::string &privacy,
      const std::string &title,
      const std::string &description,
      const std::string &app_attribution_tag,
      const OnFailed &on_failed,
      const OnLiveVideoPosted &on_live_video_posted) = 0;
};


class StreamingServiceProvider::StreamServer {
 public:
  StreamServer(
      const std::string &id_,
      const std::string &name,
      const std::string &url,
      const std::string &availability);
  virtual ~StreamServer();

  const std::string &id() const { return id_; }
  const std::string &name() const { return name_; }
  const std::string &url() const { return url_; }
  const std::string &availability() const { return availability_; }

  boost::property_tree::ptree ToTree() const;

 private:
  const std::string id_;
  const std::string name_;
  const std::string url_;
  const std::string availability_;
};


class StreamingServiceProvider::UserPage {
 public:
  UserPage(
      const std::string &id,
      const std::string &name,
      const std::string &link,
      const std::string &access_token);
  virtual ~UserPage();

  const std::string &id() const { return id_; }
  const std::string &name() const { return name_; }
  const std::string &link() const { return link_; }
  const std::string &access_token() const { return access_token_; }

  boost::property_tree::ptree ToTree() const;

 private:
  const std::string id_;
  const std::string name_;
  const std::string link_;
  const std::string access_token_;
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_STREAMING_SERVICE_STREAMING_SERVICE_PROVIDER_H_
