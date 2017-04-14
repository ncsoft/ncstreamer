/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_STREAMING_SERVICE_FACEBOOK_API_H_
#define NCSTREAMER_CEF_SRC_STREAMING_SERVICE_FACEBOOK_API_H_


#include <string>
#include <vector>

#include "boost/property_tree/ptree.hpp"

#include "ncstreamer_cef/src/lib/uri.h"


namespace ncstreamer {
class FacebookApi {
 public:
  class Login;
  class Graph;

 private:
  static const char *kScheme;
  static const char *kVersion;
};


class FacebookApi::Login {
 public:
  class Oauth;
  class Redirect;

 private:
  static const char *kAuthority;
};


class FacebookApi::Login::Oauth {
 public:
  static Uri BuildUri(
      const std::string &client_id,
      const Uri &redirect_uri,
      const std::string &response_type,
      const std::string &display,
      const std::vector<std::string> &scope);

 private:
  static const std::string &static_path();
};


class FacebookApi::Login::Redirect {
 public:
  static const Uri &static_uri();

  static std::string ExtractAccessToken(const Uri::Query &query);
};


class FacebookApi::Graph {
 public:
  class Me;
  class LiveVideos;

 private:
  static const char *kAuthority;
};


class FacebookApi::Graph::Me {
 public:
  static const Uri &static_uri();
  static Uri BuildUri(
      const std::string &access_token,
      const std::vector<std::string> &fields);

 private:
  static const std::string &static_path();
};


class FacebookApi::Graph::LiveVideos {
 public:
  static Uri BuildUri(
      const std::string &user_page_id);

  static boost::property_tree::ptree BuildPostContent(
      const std::string &access_token,
      const std::string &privacy,
      const std::string &title,
      const std::string &description);

 private:
  static std::string BuildPath(
      const std::string &user_page_id);
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_STREAMING_SERVICE_FACEBOOK_API_H_
