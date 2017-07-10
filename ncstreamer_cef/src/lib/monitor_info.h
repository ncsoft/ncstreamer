/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#ifndef NCSTREAMER_CEF_SRC_LIB_MONITOR_INFO_H_
#define NCSTREAMER_CEF_SRC_LIB_MONITOR_INFO_H_


#include "ncstreamer_cef/src/lib/rectangle.h"
#include "windows.h"  // NOLINT


namespace ncstreamer {
class MonitorInfo {
 public:
  static bool RectInMonitor(const Rectangle &rectangle);

 private:
  static BOOL CALLBACK MonitorEnumProc(
      HMONITOR monitor, HDC hdc, LPRECT rect, LPARAM data);
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_LIB_MONITOR_INFO_H_
