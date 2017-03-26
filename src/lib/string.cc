/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/lib/string.h"

#include <sstream>


namespace ncstreamer {
std::wstring String::Join(
    const std::vector<std::wstring> &elems,
    const std::wstring &separator) {
  if (elems.empty() == true) {
    return L"";
  }

  std::wstringstream ss;
  auto i = elems.begin();
  ss << *i;
  for (++i; i != elems.end(); ++i) {
    ss << separator << *i;
  }
  return ss.str();
}
}  // namespace ncstreamer
