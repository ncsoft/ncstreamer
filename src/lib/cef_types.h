/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_LIB_CEF_TYPES_H_
#define SRC_LIB_CEF_TYPES_H_


#include "include/cef_base.h"

#include "src/lib/rectangle.h"


namespace ncstreamer {
class CefWindowRectangle {
 public:
  static void Reset(const Rectangle &rect, CefWindowInfo *window_info);
};
}  // namespace ncstreamer


#endif  // SRC_LIB_CEF_TYPES_H_
