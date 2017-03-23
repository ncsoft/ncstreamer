/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_LIB_URI_H_
#define SRC_LIB_URI_H_


#include <string>
#include <unordered_map>
#include <utility>
#include <vector>


namespace ncstreamer {
class Uri {
 public:
  using QueryParamVector = std::vector<std::pair<std::wstring, std::wstring>>;

  class Query {
   public:
    explicit Query(const std::wstring &query_string);
    Query();
    virtual ~Query();

    const std::wstring &query_string() const { return query_string_; }
    const std::wstring &GetParameter(const std::wstring &key) const;

   private:
    std::wstring query_string_;
    std::unordered_map<std::wstring, std::wstring> params_;
  };

  static std::wstring ToString(
      const std::wstring &scheme,
      const std::wstring &authority,
      const std::wstring &path,
      const QueryParamVector &query,
      const std::wstring &fragment);

  static std::wstring ToString(
      const std::wstring &scheme,
      const std::wstring &authority,
      const std::wstring &path,
      const QueryParamVector &query);

  static std::wstring ToString(
      const std::wstring &scheme,
      const std::wstring &authority,
      const std::wstring &path);

 private:
  static std::wstring ToString(const QueryParamVector &query);

  static std::wstring Encode(const std::wstring &raw);
};
}  // namespace ncstreamer


#endif  // SRC_LIB_URI_H_
