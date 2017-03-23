/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_LIB_URI_H_
#define SRC_LIB_URI_H_


#include <string>
#include <utility>
#include <vector>


namespace ncstreamer {
class Uri {
 public:
  using QueryParameter = std::pair<std::wstring, std::wstring>;
  using Query = std::vector<QueryParameter>;

  static std::wstring ToString(
      const std::wstring &scheme,
      const std::wstring &authority,
      const std::wstring &path,
      const Query &query);

 private:
  static std::wstring ToString(const Query &query);
  static std::wstring Encode(const std::wstring &raw);
};
}  // namespace ncstreamer


#endif  // SRC_LIB_URI_H_
