/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_LIB_DIMENSION_H_
#define SRC_LIB_DIMENSION_H_


namespace ncstreamer {
template <typename T>
class Dimension {
 public:
  Dimension(T width, T height)
      : width_{width}, height_{height} {}

  T width() const { return width_; }
  T height() const { return height_; }
  bool empty() const { return width_ == 0 && height_ == 0; }

  void set_width(T width) { width_ = width; }
  void set_height(T height) { height_ = height; }

 private:
  T width_;
  T height_;
};
}  // namespace ncstreamer


#endif  // SRC_LIB_DIMENSION_H_
