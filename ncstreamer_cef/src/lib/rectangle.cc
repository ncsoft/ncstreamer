/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/lib/rectangle.h"


namespace ncstreamer {
Rectangle Rectangle::Center(int width, int height) const {
  return Rectangle{
      x_ + ((width_ - width) / 2),
      y_ + ((height_ - height) / 2),
      width,
      height};
}


Rectangle Rectangle::Center(const Dimension<int> &dimension) const {
  return Center(dimension.width(), dimension.height());
}
}  // namespace ncstreamer
