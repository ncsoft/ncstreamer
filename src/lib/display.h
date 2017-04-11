/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_LIB_DISPLAY_H_
#define SRC_LIB_DISPLAY_H_


#include "src/lib/dimension.h"
#include "src/lib/windows_types.h"


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


#endif  // SRC_LIB_DISPLAY_H_
