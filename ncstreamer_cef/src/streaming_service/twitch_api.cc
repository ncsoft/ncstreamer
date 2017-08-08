/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#include "ncstreamer_cef/src/streaming_service/twitch_api.h"

#include <sstream>
#include <utility>

#include "ncstreamer_cef/src/lib/string.h"


namespace ncstreamer {
const char *TwitchApi::kScheme{"https"};


const char *TwitchApi::kAuthority{"api.twitch.tv/kraken"};


Uri TwitchApi::Login::Oauth::Code::BuildUri(
    const std::string &client_id,
    const Uri &redirect_uri,
    const std::vector<std::string> &scope) {
  return {kScheme, kAuthority, static_path(), Uri::Query{{
      {"client_id", client_id},
      {"redirect_uri", redirect_uri.uri_string()},
      {"response_type", "code"},
      {"scope", String::Join(scope, " ")}}}};
}


const std::string &TwitchApi::Login::Oauth::Code::static_path() {
  static const std::string kPath{[]() {
    std::stringstream ss;
    ss << "/oauth2/authorize";
    return ss.str();
  }()};
  return kPath;
}


Uri TwitchApi::Login::Oauth::Token::BuildUri(
    const std::string &client_id,
    const std::string &client_secret,
    const std::string &code,
    const Uri &redirect_uri) {
  return {kScheme, kAuthority, static_path(), Uri::Query{{
      {"client_id", client_id},
      {"client_secret", client_secret},
      {"code", code},
      {"grant_type", "authorization_code"},
      {"redirect_uri", redirect_uri.uri_string()}}}};
}


const std::string &TwitchApi::Login::Oauth::Token::static_path() {
  static const std::string kPath{[]() {
    std::stringstream ss;
    ss << "/oauth2/token";
    return ss.str();
  }()};
  return kPath;
}


const Uri &TwitchApi::Login::Redirect::static_uri() {
  static const Uri kUri{"http://localhost/"};
  return kUri;
}


std::string TwitchApi::Login::Redirect::ExtractCode(
    const Uri::Query &query) {
  return query.GetParameter("code");
}


Uri TwitchApi::Graph::User::BuildUri(
  const std::string &client_id,
  const std::string &access_token) {
  return {kScheme, kAuthority, static_path(), Uri::Query{{
      {"client_id", client_id},
      {"oauth_token", access_token}}}};
}


const std::string &TwitchApi::Graph::User::static_path() {
  static const std::string kPath{[]() {
    std::stringstream ss;
    ss << "/user";
    return ss.str();
  }()};
  return kPath;
}


Uri TwitchApi::Graph::Channel::BuildUri(
    const std::string &client_id,
    const std::string &access_token) {
  return {kScheme, kAuthority, static_path(), Uri::Query{{
      {"client_id", client_id},
      {"oauth_token", access_token}}}};
}


const std::string &TwitchApi::Graph::Channel::static_path() {
  static const std::string kPath{[]() {
    std::stringstream ss;
    ss << "/channel";
    return ss.str();
  }()};
  return kPath;
}


Uri TwitchApi::Graph::UpdateChannel::BuildUri(
    const std::string &channel_id,
    const std::string &access_token) {
  return {kScheme, kAuthority, BuildPath(channel_id), Uri::Query{{
      {"oauth_token", access_token},
      {"api_version", "5"}}}};
}


boost::property_tree::ptree
    TwitchApi::Graph::UpdateChannel::BuildPostContent(
        const std::string &description,
        const std::string &game,
        const bool &channel_feed_enabled) {
  boost::property_tree::ptree channel_content;
  channel_content.add<std::string>(
      "status", description);
  channel_content.add<std::string>(
      "game", game);
  channel_content.add<bool>(
      "channel_feed_enabled", channel_feed_enabled);

  boost::property_tree::ptree post_content;
  post_content.add_child("channel", channel_content);
  return std::move(post_content);
}


std::string TwitchApi::Graph::UpdateChannel::BuildPath(
    const std::string &channel_id) {
  std::stringstream ss;
  ss << "/channels" << "/" << channel_id;
  return ss.str();
}
}  // namespace ncstreamer
