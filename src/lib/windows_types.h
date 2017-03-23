/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_LIB_WINDOWS_TYPES_H_
#define SRC_LIB_WINDOWS_TYPES_H_


#include "windows.h"  // NOLINT

#include "src/lib/rectangle.h"


namespace ncstreamer {
class Windows {
 public:
  static Rectangle GetWindowRectangle(HWND wnd);
};
}  // namespace ncstreamer


#endif  // SRC_LIB_WINDOWS_TYPES_H_
