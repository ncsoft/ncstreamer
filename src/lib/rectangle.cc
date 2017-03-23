/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/lib/rectangle.h"


namespace ncstreamer {
Rectangle Rectangle::Center(int width, int height) const {
  return Rectangle{
      x_ + ((width_ - width) / 2),
      y_ + ((height_ - height) / 2),
      width,
      height};
}
}  // namespace ncstreamer
