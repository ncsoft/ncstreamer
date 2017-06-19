/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#include <iostream>

#include "static_ui_generator/src/program_option_map.h"
#include "static_ui_generator/src/ui_generator.h"


int main(int argc, char *argv[]) {
  static_ui_generator::ProgramOptionMap options{argc, argv};
  if (options.failed_to_parse()) {
    std::cout << options.description();
    return -1;
  }
  if (options.help()) {
    std::cout << options.description();
    return 0;
  }

  const std::string &err_msg = static_ui_generator::Run(
      options.texts_file(),
      options.input_dir(),
      options.output_dir(),
      &std::cout);
  if (err_msg.empty() == false) {
    std::cerr << err_msg << std::endl;
    return -1;
  }

  return 0;
}
