/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#ifndef STATIC_UI_GENERATOR_SRC_UI_GENERATOR_H_
#define STATIC_UI_GENERATOR_SRC_UI_GENERATOR_H_


#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "boost/filesystem/path.hpp"


namespace static_ui_generator {
using ContentsVector = std::vector<std::pair<
    boost::filesystem::path /*filename*/, std::string /*contents*/>>;

extern std::string Run(
    const std::string &texts_file,
    const std::string &input_dir,
    const std::string &output_dir,
    std::ostream *info_out);

extern ContentsVector ReadTemplates(
    const std::string &input_dir);
}  // namespace static_ui_generator


#endif  // STATIC_UI_GENERATOR_SRC_UI_GENERATOR_H_
