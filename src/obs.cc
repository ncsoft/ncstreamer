/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/obs.h"

#include "obs-studio/libobs/obs.h"


namespace ncstreamer {
void Obs::SetUp() {
  obs_startup("en-US", nullptr, nullptr);
  obs_load_all_modules();
}


void Obs::ShutDown() {
  obs_shutdown();
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
    titles.emplace_back(std::string(val));
  }

  obs_properties_destroy(props);
  obs_source_release(source);
  return titles;
}
}  // namespace ncstreamer
