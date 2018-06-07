/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/lib/named_mutex.h"


namespace ncstreamer {
NamedMutex::NamedMutex(const wchar_t *name)
    : name_{name},
      mutex_handle_{nullptr} {
}


NamedMutex::~NamedMutex() {
  UnregisterMutex();
}


bool NamedMutex::IsMutexRegistered() const {
  bool result = false;
  try {
    HANDLE mtx = ::CreateMutex(nullptr, false, name_);
    DWORD err = ::GetLastError();
    if (mtx == nullptr) {
      result = false;
    } else {
      ::CloseHandle(mtx);
      if (err == ERROR_ALREADY_EXISTS) {
        result = true;
      } else {
        result = false;
      }
    }
  } catch (...) {
    result = false;
  }
  return result;
}


bool NamedMutex::RegisterMutex() {
  bool result = false;
  try {
    if (mutex_handle_ != nullptr) {
      UnregisterMutex();
    }
    mutex_handle_ = ::CreateMutex(nullptr, false, name_);
    if (mutex_handle_ == nullptr) {
      result = false;
    } else {
      result = true;
    }
  } catch (...) {
    result = false;
  }
  return result;
}


void NamedMutex::UnregisterMutex() {
  if (nullptr == mutex_handle_) {
    return;
  }
  ::CloseHandle(mutex_handle_);
}
}  // namespace ncstreamer
