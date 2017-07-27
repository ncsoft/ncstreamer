/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_LOCAL_STORAGE_H_
#define NCSTREAMER_CEF_SRC_LOCAL_STORAGE_H_


#include <string>

#include "boost/optional/optional.hpp"
#include "boost/property_tree/ptree.hpp"

#include "ncstreamer_cef/src/lib/position.h"


namespace ncstreamer {
class LocalStorage {
 public:
  static void SetUp(const std::wstring &storage_path);
  static void ShutDown();
  static LocalStorage *Get();

  std::string GetUserPage() const;
  std::string GetPrivacy() const;
  std::string GetDesignatedUser() const;
  boost::optional<Position<int>> GetWindowPosition() const;

  void SetUserPage(const std::string &user_page);
  void SetPrivacy(const std::string &privacy);
  void SetDesignatedUser(const std::string &designated_user);
  void SetWindowPosition(const Position<int> &window_position);

 private:
  explicit LocalStorage(const std::wstring &storage_path);
  virtual ~LocalStorage();

  static boost::property_tree::ptree LoadFromFile(
      const std::wstring &file_path);

  static void SaveToFile(
      const boost::property_tree::ptree &tree,
      const std::wstring &file_path);

  template<typename T>
      void SetValue(const std::string &key, const T &value);

  void SetValue(
      const std::string &key,
      const boost::property_tree::ptree &value);

  static const char *kUserPage;
  static const char *kPrivacy;
  static const char *kDesignatedUser;
  static const char *kWindowPosition;
  static const char *kWindowPositionX;
  static const char *kWindowPositionY;

  static LocalStorage *static_instance;

  std::wstring storage_path_;
  boost::property_tree::ptree storage_;
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_LOCAL_STORAGE_H_
