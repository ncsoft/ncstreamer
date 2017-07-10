/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#include "ncstreamer_cef/src/lib/monitor_info.h"


namespace ncstreamer {
bool MonitorInfo::RectInMonitor(const Rectangle &rectangle) {
  BOOL ret = EnumDisplayMonitors(
      nullptr, nullptr, MonitorEnumProc, (LPARAM) &rectangle);
  return !ret;
}


BOOL CALLBACK MonitorInfo::MonitorEnumProc(
  HMONITOR monitor, HDC hdc, LPRECT rect, LPARAM data) {
  Rectangle *rectangle = (Rectangle *)data;
  RECT window_rect{rectangle->x(),
                   rectangle->y(),
                   rectangle->x() + rectangle->width(),
                   rectangle->y() + rectangle->height()};
  RECT intersect_rect;
  IntersectRect(&intersect_rect, rect, &window_rect);
  return !EqualRect(&intersect_rect, &window_rect);
}
}  // namespace ncstreamer
