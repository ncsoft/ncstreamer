/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_OBS_OBS_SOURCE_INFO_H_
#define SRC_OBS_OBS_SOURCE_INFO_H_


#include <string>


namespace ncstreamer {
class ObsSourceInfo {
 public:
  explicit ObsSourceInfo(const std::string &source);
  virtual ~ObsSourceInfo();

  const std::string &source() const { return source_; }
  const std::string &title() const { return title_; }
  const std::string &clazz() const { return clazz_; }
  const std::string &exe_name() const { return exe_name_; }

 private:
  const std::string source_;
  std::string title_;
  std::string clazz_;
  std::string exe_name_;
};
}  // namespace ncstreamer


#endif  // SRC_OBS_OBS_SOURCE_INFO_H_
