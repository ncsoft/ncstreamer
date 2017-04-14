/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/lib/string.h"

#include <sstream>


namespace ncstreamer {
std::string String::Join(
    const std::vector<std::string> &elems,
    const std::string &separator) {
  if (elems.empty() == true) {
    return "";
  }

  std::stringstream ss;
  auto i = elems.begin();
  ss << *i;
  for (++i; i != elems.end(); ++i) {
    ss << separator << *i;
  }
  return ss.str();
}


std::string String::EscapeSingleQuote(
    const std::string &origin) {
  std::stringstream ss;
  for (const auto &c : origin) {
    switch (c) {
      case '\\':
      case '\'': ss << '\\'; break;
      default: break;
    }
    ss << c;
  }
  return ss.str();
}
}  // namespace ncstreamer
