/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#ifndef NCSTREAMER_CEF_SRC_STREAMING_SERVICE_YOUTUBE_API_H_
#define NCSTREAMER_CEF_SRC_STREAMING_SERVICE_YOUTUBE_API_H_


#include <string>
#include <vector>

#include "boost/property_tree/ptree.hpp"

#include "ncstreamer_cef/src/lib/uri.h"


namespace ncstreamer {
class YouTubeApi {
 public:
  class Login;
  class Graph;

 private:
  static const char *kScheme;
};


class YouTubeApi::Login {
 public:
  class Oauth;
  class Redirect;
};


class YouTubeApi::Login::Oauth {
 public:
  class Code;
  class RefreshToken;
  class Token;

 private:
  static const char *kAuthority;
};


class YouTubeApi::Login::Oauth::Code {
 public:
  static Uri BuildUri(
      const std::string &client_id,
      const Uri &redirect_uri,
      const std::vector<std::string> &scope);

 private:
  static const std::string &static_path();
};


class YouTubeApi::Login::Oauth::RefreshToken {
 public:
  static Uri BuildUri();

 private:
  static const std::string &static_path();
};


class YouTubeApi::Login::Oauth::Token {
 public:
  static Uri BuildUri();

 private:
  static const std::string &static_path();
  static const char *kAuthority;
};


class YouTubeApi::Login::Redirect {
 public:
  static const Uri &static_uri();

  static std::string ExtractAccessCode(const Uri::Query &query);
};


class YouTubeApi::Graph {
 public:
  class Channel;
  class BroadcastList;
  class StreamList;
  class VideoUpdate;

 private:
  static const char *kAuthority;
};


class YouTubeApi::Graph::Channel {
 public:
  static Uri BuildUri(
      const std::string &access_token);

 private:
  static const std::string &static_path();
};


class YouTubeApi::Graph::BroadcastList {
 public:
  static Uri BuildUri(
      const std::string &access_token);

 private:
  static const std::string &static_path();
};


class YouTubeApi::Graph::StreamList {
 public:
  static Uri BuildUri(
      const std::string &steam_id,
      const std::string &access_token);

 private:
  static const std::string &static_path();
};


class YouTubeApi::Graph::VideoUpdate {
 public:
  static Uri BuildUri(
      const std::string &access_token);

  static boost::property_tree::ptree BuildPostContent(
      const std::string &broadcast_id,
      const std::string &title,
      const std::string &privacy_status);

 private:
  static const std::string &static_path();
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_STREAMING_SERVICE_YOUTUBE_API_H_
