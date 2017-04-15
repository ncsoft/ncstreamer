/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_LIB_DISPLAY_H_
#define NCSTREAMER_CEF_SRC_LIB_DISPLAY_H_


#include "ncstreamer_cef/src/lib/dimension.h"
#include "ncstreamer_cef/src/lib/windows_types.h"


namespace ncstreamer {
class Display {
 public:
  /// @return Scaled size from the given base size.
  static Dimension<int> Scale(const Dimension<int> &base_size);
  static RECT Scale(const RECT &base_size);

 private:
  static Dimension<int> GetDpi();
  static Dimension<int> Scale(
      const Dimension<int> &dpi,
      const Dimension<int> &base_size);
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_LIB_DISPLAY_H_
