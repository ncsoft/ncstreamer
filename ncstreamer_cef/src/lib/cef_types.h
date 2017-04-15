/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_LIB_CEF_TYPES_H_
#define NCSTREAMER_CEF_SRC_LIB_CEF_TYPES_H_


#include "include/cef_base.h"

#include "ncstreamer_cef/src/lib/rectangle.h"


namespace ncstreamer {
class CefWindowRectangle {
 public:
  static void Reset(const Rectangle &rect, CefWindowInfo *window_info);
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_LIB_CEF_TYPES_H_
