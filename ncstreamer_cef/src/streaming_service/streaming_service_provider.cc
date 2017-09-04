/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/streaming_service/streaming_service_provider.h"

#include <utility>


namespace ncstreamer {
StreamingServiceProvider::StreamingServiceProvider() {
}


StreamingServiceProvider::~StreamingServiceProvider() {
}


StreamingServiceProvider::StreamServer::StreamServer(
    const std::string &id,
    const std::string &name,
    const std::string &url,
    const std::string &availability)
    : id_{id},
      name_{name},
      url_{url},
      availability_{availability} {
}


StreamingServiceProvider::StreamServer::~StreamServer() {
}


boost::property_tree::ptree
    StreamingServiceProvider::StreamServer::ToTree() const {
  boost::property_tree::ptree tree;
  tree.put("id", id_);
  tree.put("name", name_);
  tree.put("url", url_);
  tree.put("availability", availability_);
  return std::move(tree);
}


StreamingServiceProvider::UserPage::UserPage(
    const std::string &id,
    const std::string &name,
    const std::string &link,
    const std::string &access_token)
    : id_{id},
      name_{name},
      link_{link},
      access_token_{access_token} {
}


StreamingServiceProvider::UserPage::~UserPage() {
}


boost::property_tree::ptree
    StreamingServiceProvider::UserPage::ToTree() const {
  boost::property_tree::ptree tree;
  tree.put("id", id_);
  tree.put("name", name_);
  tree.put("link", link_);
  tree.put("access_token", access_token_);
  return std::move(tree);
}
}  // namespace ncstreamer
