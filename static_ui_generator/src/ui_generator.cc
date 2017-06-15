/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#include "static_ui_generator/src/ui_generator.h"

#include <codecvt>
#include <fstream>
#include <locale>
#include <regex>  // NOLINT
#include <unordered_map>
#include <vector>

#include "boost/algorithm/string/replace.hpp"
#include "boost/filesystem.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/property_tree/ptree.hpp"


namespace static_ui_generator {
std::string Run(
    const std::string &texts_file,
    const std::string &input_dir,
    const std::string &output_dir,
    std::ostream *info_out) {
  std::string err_msg{};

  // read texts file
  boost::property_tree::ptree texts_all;
  try {
    boost::property_tree::read_json(texts_file, texts_all);
  } catch (const std::exception &e) {
    err_msg = e.what();
  }
  if (!err_msg.empty()) {
    return err_msg;
  }
  *info_out << "Target locales: " << texts_all.size() << std::endl;

  try {
    // locale loop
    for (const auto &prop : texts_all) {
      auto locale = prop.first;
      auto texts = prop.second;

      // file loop
      for (boost::filesystem::directory_iterator i{input_dir};
           i != boost::filesystem::directory_iterator(); ++i) {
        if (!is_regular_file(i->status())) {
          continue;
        }
        static const std::regex kPattern{R"(%([_A-Z][_A-Z0-9]*)%)"};
        boost::filesystem::ifstream ifs{i->path().c_str()};
        std::string contents{
            std::istreambuf_iterator<char>{ifs},
            std::istreambuf_iterator<char>{}};
        ifs.close();
        std::string output_contents = contents;

        // replace loop
        std::unordered_map<std::string, std::string> key_map;
        std::sregex_iterator words_begin{
            contents.begin(), contents.end(), kPattern};
        for (std::sregex_iterator j = words_begin;
             j != std::sregex_iterator(); ++j) {
          const std::smatch &matched = *j;
          key_map.emplace(matched.str(), matched[1].str());
        }
        for (const auto &key : key_map) {
          std::string &text = texts.get<std::string>(key.second);
          const std::string &extension = i->path().extension().string();
          if (extension == ".js") {
            static const std::unordered_map<std::string,
                                            std::string> kEscapeChars{
               {"\r", "\\r"},
               {"\n", "\\n"},
               {"'", "\\'"},
               {"\"", "\\\""}};
            for (auto escape : kEscapeChars) {
              boost::replace_all(text, escape.first, escape.second);
            }
          }
          boost::replace_all(output_contents, key.first, text);
        }

        // create file
        const boost::filesystem::path dir{
            boost::filesystem::path{output_dir} /  // NOLINT
            boost::filesystem::path{locale}};      // NOLINT
        if (boost::filesystem::exists(dir) ||
            boost::filesystem::create_directories(dir)) {
          const boost::filesystem::path file_name =
              dir / i->path().filename().c_str();
          boost::filesystem::ofstream ofs{file_name};
          ofs << output_contents;
          ofs.close();
        }
      }

      *info_out << "Done: " << locale << std::endl;
    }
  } catch (const std::exception &e) {
    err_msg = e.what();
  }
  return err_msg;
}
}  // namespace static_ui_generator
