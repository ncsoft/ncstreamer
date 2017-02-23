/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_OBS_H_
#define SRC_OBS_H_


#include <fstream>
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

  bool SetUpLog();

  static Obs *static_instance;

  std::fstream log_file_;
};
}  // namespace ncstreamer


#endif  // SRC_OBS_H_
