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
    const std::wstring &name)
    : id_{id},
      name_{name} {
}


StreamingServiceProvider::UserPage::~UserPage() {
}


boost::property_tree::ptree
    StreamingServiceProvider::UserPage::ToTree() const {
  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

  boost::property_tree::ptree tree;
  tree.put("id", converter.to_bytes(id_));
  tree.put("name", converter.to_bytes(name_));
  return std::move(tree);
}
}  // namespace ncstreamer
