/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/lib/display.h"

#include <cmath>

#include "src/lib/windows_types.h"


namespace ncstreamer {
Dimension<int> Display::Scale(const Dimension<int> &base_size) {
  HDC screen = ::GetDC(NULL);
  if (!screen) {
    return base_size;
  }

  int dpi_x = ::GetDeviceCaps(screen, LOGPIXELSX);
  int dpi_y = ::GetDeviceCaps(screen, LOGPIXELSY);
  ::ReleaseDC(NULL, screen);

  return {::MulDiv(base_size.width(), dpi_x, 96),
          ::MulDiv(base_size.height(), dpi_y, 96)};
}
}  // namespace ncstreamer
