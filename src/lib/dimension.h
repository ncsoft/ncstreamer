/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_LIB_DIMENSION_H_
#define SRC_LIB_DIMENSION_H_


namespace ncstreamer {
class Dimension {
 public:
  Dimension(int width, int height)
      : width_{width}, height_{height} {}

  int width() const { return width_; }
  int height() const { return height_; }
  bool empty() const { return width_ == 0 && height_ == 0; }

  void set_width(int width) { width_ = width; }
  void set_height(int height) { height_ = height; }

 private:
  int width_;
  int height_;
};
}  // namespace ncstreamer


#endif  // SRC_LIB_DIMENSION_H_
