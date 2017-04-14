/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/lib/windows_types.h"


namespace ncstreamer {
Rectangle Windows::GetWindowRectangle(HWND wnd) {
  RECT r;
  ::GetWindowRect(wnd, &r);

  return Rectangle{
      r.left,
      r.top,
      r.right - r.left,
      r.bottom - r.top};
}
}  // namespace ncstreamer
