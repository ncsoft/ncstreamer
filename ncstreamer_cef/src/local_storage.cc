/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/local_storage.h"

#include <cassert>
#include <fstream>

#include "boost/property_tree/json_parser.hpp"


namespace ncstreamer {
void LocalStorage::SetUp(const std::wstring &storage_path) {
  assert(!static_instance);
  static_instance = new LocalStorage{storage_path};
}


void LocalStorage::ShutDown() {
  assert(static_instance);
  delete static_instance;
  static_instance = nullptr;
}


LocalStorage *LocalStorage::Get() {
  assert(static_instance);
  return static_instance;
}


LocalStorage::LocalStorage(const std::wstring &storage_path)
    : storage_path_{storage_path},
      storage_{LoadFromFile(storage_path)} {
}


LocalStorage::~LocalStorage() {
}


boost::property_tree::ptree LocalStorage::LoadFromFile(
    const std::wstring &file_path) {
  boost::property_tree::ptree tree;
  std::ifstream fs{file_path};
  try {
    boost::property_tree::json_parser::read_json(fs, tree);
  } catch (...) {
    tree.clear();
  }
  return tree;
}


void LocalStorage::SaveToFile(
    const boost::property_tree::ptree &tree,
    const std::wstring &file_path) {
  std::ofstream fs{file_path};
  boost::property_tree::write_json(fs, tree);
}


template<typename T>
    void LocalStorage::SetValue(const std::string &key, const T &value) {
  storage_.put(key, value);
  SaveToFile(storage_, storage_path_);
}


template
void LocalStorage::SetValue<std::string>(
    const std::string &key, const std::string &value);
template
void LocalStorage::SetValue<int>(
    const std::string &key, const int &value);
template
void LocalStorage::SetValue<bool>(
    const std::string &key, const bool &value);


LocalStorage *LocalStorage::static_instance{nullptr};
}  // namespace ncstreamer
