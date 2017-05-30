/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_DESIGNATED_USER_H_
#define NCSTREAMER_CEF_SRC_DESIGNATED_USER_H_


#include <string>


namespace ncstreamer {
class DesignatedUser {
 public:
  static void SetUp(const std::wstring &designated_user);
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_DESIGNATED_USER_H_
