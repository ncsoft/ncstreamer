/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/js_executor.h"

#include <sstream>

#ifdef _MSC_VER
#pragma warning(disable: 4819)
#endif
#include "boost/property_tree/json_parser.hpp"
#ifdef _MSC_VER
#pragma warning(default: 4819)
#endif
#include "boost/property_tree/ptree.hpp"


namespace ncstreamer {
void JsExecutor::ExecuteAngularJs(
    CefRefPtr<CefBrowser> browser,
    const std::string &controller,
    const std::string &func_name,
    const std::string &arg_name,
    const std::vector<std::string> &arg_value) {
  std::stringstream js;
  js << "angular.element('[ng-controller=" << controller << "]').scope().";

  boost::property_tree::ptree arg_value_tree;
  for (const auto &elem : arg_value) {
    boost::property_tree::ptree elem_node;
    elem_node.put("", elem);
    arg_value_tree.push_back({"", elem_node});
  }

  boost::property_tree::ptree arg_value_root;
  arg_value_root.add_child(arg_name, arg_value_tree);

  js << func_name << "(";
  boost::property_tree::write_json(js, arg_value_root, false);
  js << ")";

  browser->GetMainFrame()->ExecuteJavaScript(js.str(), "", 0);
}
}  // namespace ncstreamer
