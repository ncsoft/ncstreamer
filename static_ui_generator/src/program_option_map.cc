/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#include "static_ui_generator/src/program_option_map.h"


namespace static_ui_generator {
ProgramOptionMap::ProgramOptionMap(int argc, const char *const argv[])
    : failed_to_parse_{false},
      description_{"Options"},
      help_{false},
      texts_file_{},
      input_dir_{},
      output_dir_{} {
  description_.add_options()
      ("help", "Help screen")
      ("texts",
        boost::program_options::value<std::string>()->default_value(
            "./localized_texts.json"),
        "Json file")
      ("input-dir",
        boost::program_options::value<std::string>()->default_value("./"),
        "Directory of input files")
      ("output-dir",
        boost::program_options::value<std::string>()->default_value("./"),
        "Directory of output files");

  boost::program_options::variables_map vm;
  try {
    boost::program_options::store(
        parse_command_line(argc, argv, description_), vm);
  } catch (const boost::program_options::error &/*e*/) {
    failed_to_parse_ = true;
  }

  if (failed_to_parse_ == true) {
    return;
  }

  boost::program_options::notify(vm);
  help_ = (vm.count("help") >= 1);

  texts_file_ = vm["texts"].as<std::string>();
  input_dir_ = vm["input-dir"].as<std::string>();
  output_dir_ = vm["output-dir"].as<std::string>();
}


bool ProgramOptionMap::failed_to_parse() const {
  return failed_to_parse_;
}


const boost::program_options::options_description &
    ProgramOptionMap::description() const {
  return description_;
}


bool ProgramOptionMap::help() const {
  return help_;
}


const std::string &ProgramOptionMap::texts_file() const {
  return texts_file_;
}


const std::string &ProgramOptionMap::input_dir() const {
  return input_dir_;
}


const std::string &ProgramOptionMap::output_dir() const {
  return output_dir_;
}


ProgramOptionMap::~ProgramOptionMap() {
}
}  // namespace static_ui_generator
