/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#ifndef STATIC_UI_GENERATOR_SRC_PROGRAM_OPTION_MAP_H_
#define STATIC_UI_GENERATOR_SRC_PROGRAM_OPTION_MAP_H_


#include <string>
#include "boost/program_options.hpp"


namespace static_ui_generator {
class ProgramOptionMap {
 public:
  ProgramOptionMap(int argc, const char *const argv[]);
  virtual ~ProgramOptionMap();

  bool failed_to_parse() const;
  const boost::program_options::options_description &description() const;
  bool help() const;

  const std::string &texts_file() const;
  const std::string &input_dir() const;
  const std::string &output_dir() const;

 private:
  bool failed_to_parse_;
  boost::program_options::options_description description_;
  bool help_;

  std::string texts_file_;
  std::string input_dir_;
  std::string output_dir_;
};
}  // namespace static_ui_generator


#endif  // STATIC_UI_GENERATOR_SRC_PROGRAM_OPTION_MAP_H_
