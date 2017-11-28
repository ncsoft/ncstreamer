/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#ifndef NCSTREAMER_CEF_SRC_STREAMING_SERVICE_TWITCH_API_H_
#define NCSTREAMER_CEF_SRC_STREAMING_SERVICE_TWITCH_API_H_


#include <string>
#include <vector>

#include "boost/property_tree/ptree.hpp"

#include "ncstreamer_cef/src/lib/uri.h"


namespace ncstreamer {
class TwitchApi {
 public:
  class Login;
  class Graph;

 private:
  static const char *kScheme;
  static const char *kAuthority;
};


class TwitchApi::Login {
 public:
  class Oauth;
  class Redirect;
};


class TwitchApi::Login::Oauth {
 public:
  class Code;
  class Token;
};


class TwitchApi::Login::Oauth::Code {
 public:
  static Uri BuildUri(
      const std::string &client_id,
      const Uri &redirect_uri,
      const std::vector<std::string> &scope);

 private:
  static const std::string &static_path();
};


class TwitchApi::Login::Oauth::Token {
 public:
  static Uri BuildUri(
      const std::string &client_id,
      const std::string &client_secret,
      const std::string &code,
      const Uri &redirect_uri);

 private:
  static const std::string &static_path();
};


class TwitchApi::Login::Redirect {
 public:
  static const Uri &static_uri();

  static std::string ExtractAccessToken(const Uri::Query &query);
};


class TwitchApi::Graph {
 public:
  class User;
  class Channel;
  class Ingests;
  class UpdateChannel;
};


class TwitchApi::Graph::User {
 public:
  static Uri BuildUri(
      const std::string &client_id,
      const std::string &access_token);

 private:
  static const std::string &static_path();
};


class TwitchApi::Graph::Channel {
 public:
  static Uri BuildUri(
      const std::string &client_id,
      const std::string &access_token);

 private:
  static const std::string &static_path();
};


class TwitchApi::Graph::Ingests {
 public:
  static Uri BuildUri(
      const std::string &client_id);

 private:
  static const std::string &static_path();
};


class TwitchApi::Graph::UpdateChannel {
 public:
  static Uri BuildUri(
      const std::string &channel_id,
      const std::string &access_token);

  static boost::property_tree::ptree BuildPostContent(
      const std::string &description,
      const std::string &game,
      const bool &channel_feed_enabled);

 private:
  static std::string BuildPath(
      const std::string &user_page_id);
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_STREAMING_SERVICE_TWITCH_API_H_
