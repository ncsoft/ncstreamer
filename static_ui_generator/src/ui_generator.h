/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#ifndef STATIC_UI_GENERATOR_SRC_UI_GENERATOR_H_
#define STATIC_UI_GENERATOR_SRC_UI_GENERATOR_H_


#include <ostream>
#include <string>


namespace static_ui_generator {
extern std::string Run(
    const std::string &texts_file,
    const std::string &input_dir,
    const std::string &output_dir,
    std::ostream *info_out);
}  // namespace static_ui_generator


#endif  // STATIC_UI_GENERATOR_SRC_UI_GENERATOR_H_
