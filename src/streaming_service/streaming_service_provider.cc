/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/streaming_service/streaming_service_provider.h"

#include <codecvt>
#include <locale>
#include <utility>


namespace ncstreamer {
StreamingServiceProvider::StreamingServiceProvider() {
}


StreamingServiceProvider::~StreamingServiceProvider() {
}


StreamingServiceProvider::UserPage::UserPage(
    const std::wstring &id,
    const std::wstring &name,
    const std::wstring &link,
    const std::wstring &access_token)
    : id_{id},
      name_{name},
      link_{link},
      access_token_{access_token} {
}


StreamingServiceProvider::UserPage::~UserPage() {
}


boost::property_tree::ptree
    StreamingServiceProvider::UserPage::ToTree() const {
  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

  boost::property_tree::ptree tree;
  tree.put("id", converter.to_bytes(id_));
  tree.put("name", converter.to_bytes(name_));
  tree.put("link", converter.to_bytes(link_));
  tree.put("access_token", converter.to_bytes(access_token_));
  return std::move(tree);
}
}  // namespace ncstreamer
