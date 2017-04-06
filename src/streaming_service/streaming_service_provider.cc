/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/streaming_service/streaming_service_provider.h"

#include <utility>


namespace ncstreamer {
StreamingServiceProvider::StreamingServiceProvider() {
}


StreamingServiceProvider::~StreamingServiceProvider() {
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
