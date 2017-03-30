/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_LIB_STRING_H_
#define SRC_LIB_STRING_H_


#include <string>
#include <vector>


namespace ncstreamer {
class String {
 public:
  static std::wstring Join(
      const std::vector<std::wstring> &elems,
      const std::wstring &separator);

  static std::string EscapeSingleQuote(
      const std::string &origin);
};
}  // namespace ncstreamer


#endif  // SRC_LIB_STRING_H_
