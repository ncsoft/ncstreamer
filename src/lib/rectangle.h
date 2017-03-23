/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_LIB_RECTANGLE_H_
#define SRC_LIB_RECTANGLE_H_


namespace ncstreamer {
class Rectangle {
 public:
  Rectangle(int x, int y, int width, int height)
      : x_{x}, y_{y}, width_{width}, height_{height} {}

  int x() const { return x_; }
  int y() const { return y_; }
  int width() const { return width_; }
  int height() const { return height_; }

  void set_x(int x) { x_ = x; }
  void set_y(int y) { y_ = y; }
  void set_width(int width) { width_ = width; }
  void set_height(int height) { height_ = height; }

  Rectangle Center(int width, int height) const;

 private:
  int x_;
  int y_;
  int width_;
  int height_;
};
}  // namespace ncstreamer


#endif  // SRC_LIB_RECTANGLE_H_
