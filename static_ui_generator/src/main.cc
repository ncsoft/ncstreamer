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

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/property_tree/ptree.hpp"


int main(int argc, char *argv[]) {
  std::string texts_file, input_dir, output_dir;
  try {
    boost::program_options::options_description desc{"Options"};
    desc.add_options()
        ("help", "Help screen")
        ("texts",
         boost::program_options::value<std::string>()->
             default_value("./localized_texts.json"),
         "Json file")
        ("input-dir",
         boost::program_options::value<std::string>()->default_value("./"),
         "Directory of input files")
        ("output-dir",
         boost::program_options::value<std::string>()->default_value("./"),
         "Directory of output files");

    boost::program_options::variables_map vm;
    boost::program_options::store(parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);
    if (vm.count("help")) {
      std::cout << desc;
      return 0;
    }
    texts_file = vm["texts"].as<std::string>();
    input_dir = vm["input-dir"].as<std::string>();
    output_dir = vm["output-dir"].as<std::string>();
  } catch (const boost::program_options::error &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }

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
          const std::regex key_pattern{key.first};
          const std::string &text = texts.get<std::string>(key.second);
          output_contents =
              std::regex_replace(output_contents, key_pattern, text);
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
