/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#include "static_ui_generator/src/ui_generator.h"

#include <codecvt>
#include <fstream>
#include <locale>
#include <regex>  // NOLINT
#include <unordered_map>

#include "boost/algorithm/string/replace.hpp"
#include "boost/filesystem.hpp"
#include "boost/property_tree/json_parser.hpp"


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

  // read input dir
  const auto &templates = ReadTemplates(input_dir);
  *info_out << "Target template files: " << templates.size() << std::endl;

  boost::filesystem::path output_dir_path{output_dir};
  try {
    // locale loop
    for (const auto &prop : texts_all) {
      const auto &locale = prop.first;
      const auto &texts = prop.second;

      const auto &outputs = GenerateLocale(templates, texts);
      WriteLocale(output_dir_path, locale, outputs);

      *info_out << "Done: " << locale << std::endl;
    }
  } catch (const std::exception &e) {
    err_msg = e.what();
  }
  return err_msg;
}


ContentsVector ReadTemplates(
    const std::string &input_dir) {
  ContentsVector templates;
  for (boost::filesystem::directory_iterator i{input_dir};
       i != boost::filesystem::directory_iterator(); ++i) {
    if (!is_regular_file(i->status())) {
      continue;
    }
    const auto &file_path = i->path();
    boost::filesystem::ifstream ifs{file_path.c_str()};
    std::string contents{
        std::istreambuf_iterator<char>{ifs},
        std::istreambuf_iterator<char>{}};
    ifs.close();
    templates.emplace_back(file_path.filename(), contents);
  }
  return templates;
}


ContentsVector GenerateLocale(
    const ContentsVector &templates,
    const boost::property_tree::ptree &texts) {
  ContentsVector outputs{};

  // file loop
  for (const auto &elem : templates) {
    const auto &file_name = elem.first;
    const auto &contents = elem.second;
    const std::string &extension = file_name.extension().string();
    const std::string &output_contents =
        ReplaceTexts(extension, contents, texts);
    outputs.emplace_back(file_name, output_contents);
  }
  return outputs;
}


std::string ReplaceTexts(
    const std::string &extension,
    const std::string &contents,
    const boost::property_tree::ptree &texts) {
  std::string output_contents = contents;

  // replace loop
  std::unordered_map<std::string, std::string> key_map;
  static const std::regex kPattern{R"(%([_A-Z][_A-Z0-9]*)%)"};
  std::sregex_iterator words_begin{
      contents.begin(), contents.end(), kPattern};
  for (std::sregex_iterator i = words_begin;
       i != std::sregex_iterator(); ++i) {
    const std::smatch &matched = *i;
    key_map.emplace(matched.str(), matched[1].str());
  }
  for (const auto &key : key_map) {
    std::string &text = texts.get<std::string>(key.second);
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
    } else if (extension == ".html") {
      static const std::unordered_map<std::string,
                                      std::string> kEscapeChars{
          {"\r\n", "<br>"}};
      for (auto escape : kEscapeChars) {
        boost::replace_all(text, escape.first, escape.second);
      }
    }
    boost::replace_all(output_contents, key.first, text);
  }
  return output_contents;
}


void WriteLocale(
    const boost::filesystem::path &output_dir_path,
    const std::string &locale,
    const ContentsVector &outputs) {
  boost::filesystem::path locale_path{locale};
  const boost::filesystem::path dir{output_dir_path / locale_path};
  boost::filesystem::create_directories(dir);
  for (const auto &elem : outputs) {
    const auto &file_name = elem.first;
    const auto &output_contents = elem.second;

    // create file
    const boost::filesystem::path file_path = dir / file_name;
    boost::filesystem::ofstream ofs{file_path};
    ofs << output_contents;
    ofs.close();
  }
}
}  // namespace static_ui_generator
