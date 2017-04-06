/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/lib/display.h"

#include <cmath>

#include "ShellScalingApi.h"  // NOLINT
#include "VersionHelpers.h"  // NOLINT


namespace ncstreamer {
Dimension<int> Display::Scale(const Dimension<int> &base_size) {
  if (::IsWindows8Point1OrGreater() == false) {
    return base_size;
  }

  HMONITOR monitor = ::MonitorFromWindow(
    ::GetActiveWindow(), MONITOR_DEFAULTTONEAREST);

  UINT dpi_x{0};
  UINT dpi_y{0};
  ::GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpi_x, &dpi_y);

  return Dimension<int>(
      static_cast<int>(std::round(base_size.width() * dpi_x / 96)),
      static_cast<int>(std::round(base_size.height() * dpi_y / 96)));
}
}  // namespace ncstreamer
