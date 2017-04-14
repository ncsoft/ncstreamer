/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/obs/obs_source_info.h"

#include <vector>
#include "boost/algorithm/string.hpp"


namespace ncstreamer {
ObsSourceInfo::ObsSourceInfo(const std::string &source)
    : source_{source},
      title_{},
      clazz_{},
      exe_name_{} {
  std::vector<std::string> tokens{};
  boost::split(tokens, source, boost::is_any_of(":"));

  auto tokensSize = tokens.size();
  if (tokensSize != 3) {
    return;
  }
  title_ = tokens.at(0);
  clazz_ = tokens.at(1);
  exe_name_ = tokens.at(2);
}


ObsSourceInfo::~ObsSourceInfo() {
}
}  // namespace ncstreamer
