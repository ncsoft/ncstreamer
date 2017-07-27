/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/streaming_service/facebook_api.h"

#include <sstream>
#include <utility>

#include "ncstreamer_cef/src/lib/string.h"


namespace ncstreamer {
const char *FacebookApi::kScheme{"https"};
const char *FacebookApi::kVersion{"v2.9"};


const char *FacebookApi::Login::kAuthority{"www.facebook.com"};


Uri FacebookApi::Login::Oauth::BuildUri(
    const std::string &client_id,
    const Uri &redirect_uri,
    const std::string &response_type,
    const std::string &display,
    const std::vector<std::string> &scope) {
  return {kScheme, kAuthority, static_path(), Uri::Query{{
      {"client_id", client_id},
      {"redirect_uri", redirect_uri.uri_string()},
      {"response_type", response_type},
      {"display", display},
      {"scope", String::Join(scope, ",")}}}};
}


const std::string &FacebookApi::Login::Oauth::static_path() {
  static const std::string kPath{[]() {
    std::stringstream ss;
    ss << "/" << kVersion << "/dialog/oauth";
    return ss.str();
  }()};
  return kPath;
}


const Uri &FacebookApi::Login::Redirect::static_uri() {
  static const Uri kUri{kScheme, kAuthority, "/connect/login_success.html"};
  return kUri;
}


std::string FacebookApi::Login::Redirect::ExtractAccessToken(
    const Uri::Query &query) {
  return query.GetParameter("access_token");
}


const char *FacebookApi::Graph::kAuthority{"graph.facebook.com"};


const Uri &FacebookApi::Graph::Me::static_uri() {
  static const Uri kUri{kScheme, kAuthority, static_path()};
  return kUri;
}


Uri FacebookApi::Graph::Me::BuildUri(
    const std::string &access_token,
    const std::vector<std::string> &fields) {
  return {kScheme, kAuthority, static_path(), Uri::Query{{
      {"access_token", access_token},
      {"fields", String::Join(fields, ",")}}}};
}


const std::string &FacebookApi::Graph::Me::static_path() {
  static const std::string kPath{[]() {
    std::stringstream ss;
    ss << "/" << kVersion << "/me";
    return ss.str();
  }()};
  return kPath;
}


Uri FacebookApi::Graph::LiveVideos::BuildUri(
    const std::string &user_page_id,
    const std::string &app_attribution_tag) {
  return {kScheme, kAuthority, BuildPath(user_page_id),
      Uri::Query{{{"app_attribution_tag", app_attribution_tag}}}};
}


boost::property_tree::ptree
    FacebookApi::Graph::LiveVideos::BuildPostContent(
        const std::string &access_token,
        const std::string &privacy,
        const std::string &title,
        const std::string &description) {
  boost::property_tree::ptree post_content;
  post_content.add<std::string>(
      "access_token", access_token);

  boost::property_tree::ptree privacy_tree;
  privacy_tree.add<std::string>(
      "value", privacy);
  post_content.add_child(
      "privacy", privacy_tree);

  post_content.add<std::string>(
      "title", title);
  post_content.add<std::string>(
      "description", description);
  return std::move(post_content);
}


std::string FacebookApi::Graph::LiveVideos::BuildPath(
    const std::string &user_page_id) {
  std::stringstream ss;
  ss << "/" << user_page_id << "/live_videos";
  return ss.str();
}


Uri FacebookApi::Graph::PostId::BuildUri(
    const std::string &access_token,
    const std::string &stream_id) {
  return {kScheme, kAuthority, BuildPath(stream_id),
      Uri::Query{{{"access_token", access_token}}}};
}


std::string FacebookApi::Graph::PostId::BuildPath(
    const std::string &stream_id) {
  std::stringstream ss;
  ss << "/" << stream_id;
  return ss.str();
}
}  // namespace ncstreamer
