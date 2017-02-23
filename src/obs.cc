/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/obs.h"

#include <cassert>
#include <fstream>
#include "obs-studio/libobs/obs.h"

#include "src_imported/from_obs_studio_ui/obs-app.hpp"


namespace ncstreamer {
void Obs::SetUp() {
  assert(!static_instance);
  static_instance = new Obs{};
}


Obs::Obs() {
  // create log file.
  {
    obs_app::MakeUserDirs();

    static std::fstream static_log_file{};
    obs_app::create_log_file(static_log_file);
  }

  obs_startup("en-US", nullptr, nullptr);
  obs_load_all_modules();
}


void Obs::ShutDown() {
  assert(static_instance);
  delete static_instance;
  static_instance = nullptr;
}


Obs::~Obs() {
  obs_shutdown();
}


Obs *Obs::Get() {
  assert(static_instance);
  return static_instance;
}


std::vector<std::string> Obs::FindAllWindowsOnDesktop() {
  std::vector<std::string> titles;

  obs_source_t *source =
      obs_source_create("window_capture", "Window Capture", nullptr, nullptr);
  obs_properties_t *props = obs_source_properties(source);
  obs_property_t *prop = obs_properties_get(props, "window");

  int count = obs_property_list_item_count(prop);
  for (int i = 0; i < count; i++) {
    const char *val = obs_property_list_item_name(prop, i);
    titles.emplace_back(val);
  }

  obs_properties_destroy(props);
  obs_source_release(source);
  return titles;
}


Obs *Obs::static_instance{nullptr};
}  // namespace ncstreamer
