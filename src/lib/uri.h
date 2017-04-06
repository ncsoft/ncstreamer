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
  class Query {
   public:
    using ParamVector = std::vector<std::pair<std::string, std::string>>;

    explicit Query(const ParamVector &params);
    explicit Query(const std::string &query_string);
    Query();
    virtual ~Query();

    static std::string ToString(const ParamVector &query);

    const std::string &query_string() const { return query_string_; }
    const std::string &GetParameter(const std::string &key) const;

   private:
    static std::string Encode(const std::string &raw);

    std::string query_string_;
    std::unordered_map<std::string, std::string> params_;
  };

  class Hasher {
   public:
    std::size_t operator()(const Uri &uri) const;
  };

  Uri(const std::string &scheme,
      const std::string &authority,
      const std::string &path,
      const Query &query,
      const std::string &fragment);
  Uri(const std::string &scheme,
      const std::string &authority,
      const std::string &path,
      const Query &query);
  Uri(const std::string &scheme,
      const std::string &authority,
      const std::string &path);
  explicit Uri(const std::string &uri_string);
  virtual ~Uri();

  const std::string &scheme() const { return scheme_; }
  const std::string &authority() const { return authority_; }
  const std::string &path() const { return path_; }
  const Query &query() const { return query_; }
  const std::string &fragment() const { return fragment_; }
  const std::string &scheme_authority_path() const {
    return scheme_authority_path_;
  }
  const std::string &uri_string() const { return uri_string_; }

 private:
  static std::string ToString(
      const std::string &scheme_authority_path,
      const Query &query,
      const std::string &fragment);

  static std::string ToString(
      const std::string &scheme,
      const std::string &authority,
      const std::string &path,
      const Query &query,
      const std::string &fragment);

  std::string scheme_;
  std::string authority_;
  std::string path_;
  Query query_;
  std::string fragment_;

  std::string scheme_authority_path_;
  std::string uri_string_;
};
}  // namespace ncstreamer


#endif  // SRC_LIB_URI_H_
