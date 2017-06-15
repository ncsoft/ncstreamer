/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#include <codecvt>
#include <fstream>
#include <iostream>
#include <locale>
#include <regex>  // NOLINT
#include <unordered_map>
#include <vector>

#include "boost/algorithm/string/replace.hpp"
#include "boost/filesystem.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/property_tree/ptree.hpp"
#include "static_ui_generator/src/program_option_map.h"


int main(int argc, char *argv[]) {
  std::string texts_file, input_dir, output_dir;
  static_ui_generator::ProgramOptionMap options{argc, argv};
  if (options.failed_to_parse()) {
    std::cout << options.description();
    return -1;
  }
  if (options.help()) {
    std::cout << options.description();
    return 0;
  }
  texts_file = options.texts_file();
  input_dir = options.input_dir();
  output_dir = options.output_dir();

  try {
    // read json file
    boost::property_tree::ptree props;
    boost::property_tree::read_json(texts_file, props);

    // locale loop
    std::cout << "Target locales: " << props.size() << std::endl;
    for (const boost::property_tree::ptree::value_type &prop : props) {
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

      std::cout << "Done: " << locale << std::endl;
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
  return 0;
}
