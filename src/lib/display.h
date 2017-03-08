/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_LIB_DISPLAY_H_
#define SRC_LIB_DISPLAY_H_


namespace ncstreamer {
class Display {
 public:
  /// @return Scaled size from the given base size.
  static int Scale(int base_size);
};
}  // namespace ncstreamer


#endif  // SRC_LIB_DISPLAY_H_
