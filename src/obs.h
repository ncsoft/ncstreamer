/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_OBS_H_
#define SRC_OBS_H_


#include <string>
#include <vector>


namespace ncstreamer {
class Obs {
 public:
  static void SetUp();
  static void ShutDown();
  static Obs *Get();

  std::vector<std::string> FindAllWindowsOnDesktop();

 private:
  Obs();
  virtual ~Obs();

  static Obs *static_instance;
};
}  // namespace ncstreamer


#endif  // SRC_OBS_H_
