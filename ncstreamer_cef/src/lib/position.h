/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_LIB_POSITION_H_
#define NCSTREAMER_CEF_SRC_LIB_POSITION_H_


namespace ncstreamer {
template <typename T>
class Position {
 public:
  Position(const T &x, const T &y)
      : x_{x}, y_{y} {}

  const T &x() const { return x_; }
  const T &y() const { return y_; }

  void set_x(const T &x) { x_ = x; }
  void set_y(const T &y) { y_ = y; }

 private:
  T x_;
  T y_;
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_LIB_POSITION_H_
