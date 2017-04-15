/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_LIB_WINDOWS_TYPES_H_
#define NCSTREAMER_CEF_SRC_LIB_WINDOWS_TYPES_H_


#include "windows.h"  // NOLINT

#include "ncstreamer_cef/src/lib/rectangle.h"


namespace ncstreamer {
class Windows {
 public:
  static Rectangle GetWindowRectangle(HWND wnd);
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_LIB_WINDOWS_TYPES_H_
