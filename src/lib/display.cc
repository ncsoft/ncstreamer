/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/lib/display.h"

#include <cmath>


namespace ncstreamer {
Dimension<int> Display::Scale(const Dimension<int> &base_size) {
  return Scale(base_size, GetDpi());
}


RECT Display::Scale(const RECT &base_size) {
  const auto &dpi = GetDpi();

  const auto &left_top = Scale(dpi, {base_size.left, base_size.top});
  const auto &right_bottom = Scale(dpi, {base_size.right, base_size.bottom});

  return {left_top.width(),
          left_top.height(),
          right_bottom.width(),
          right_bottom.height()};
}


Dimension<int> Display::GetDpi() {
  HDC screen = ::GetDC(NULL);
  if (!screen) {
    return {96, 96};
  }

  int dpi_x = ::GetDeviceCaps(screen, LOGPIXELSX);
  int dpi_y = ::GetDeviceCaps(screen, LOGPIXELSY);
  ::ReleaseDC(NULL, screen);
  return {dpi_x, dpi_y};
}


Dimension<int> Display::Scale(
    const Dimension<int> &base_size,
    const Dimension<int> &dpi) {
  return {::MulDiv(base_size.width(), dpi.width(), 96),
          ::MulDiv(base_size.height(), dpi.height(), 96)};
}
}  // namespace ncstreamer
