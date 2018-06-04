/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_LIB_NAMED_MUTEX_H_
#define NCSTREAMER_CEF_SRC_LIB_NAMED_MUTEX_H_


#include "windows.h"  // NOLINT


namespace ncstreamer {
class NamedMutex {
 public:
  explicit NamedMutex(const wchar_t *name_);
  ~NamedMutex();

  bool IsMutexRegistered();
  bool RegisterMutex();

 private:
  void UnregisterMutex();

  const wchar_t *name_;
  HANDLE mutex_handle_;
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_LIB_NAMED_MUTEX_H_
