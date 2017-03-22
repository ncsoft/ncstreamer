/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/js_executor.h"

#include <sstream>
#include <utility>

#ifdef _MSC_VER
#pragma warning(disable: 4819)
#endif
#include "boost/property_tree/json_parser.hpp"
#ifdef _MSC_VER
#pragma warning(default: 4819)
#endif


namespace ncstreamer {
void JsExecutor::Execute(
    CefRefPtr<CefBrowser> browser,
    const std::string &func_name) {
  std::stringstream js;
  js << func_name << "()";

  browser->GetMainFrame()->ExecuteJavaScript(js.str(), "", 0);
}


void JsExecutor::Execute(
    CefRefPtr<CefBrowser> browser,
    const std::string &func_name,
    const int &arg_value) {
  std::stringstream js;
  js << func_name << "(" << arg_value << ")";

  browser->GetMainFrame()->ExecuteJavaScript(js.str(), "", 0);
}


void JsExecutor::Execute(
    CefRefPtr<CefBrowser> browser,
    const std::string &func_name,
    const std::string &arg_name,
    const std::vector<std::string> &arg_value) {
  std::stringstream js;
  AppendFunctionCall(func_name, arg_name, arg_value, &js);

  browser->GetMainFrame()->ExecuteJavaScript(js.str(), "", 0);
}


void JsExecutor::ExecuteAngularJs(
    CefRefPtr<CefBrowser> browser,
    const std::string &controller,
    const std::string &func_name,
    const std::string &arg_name,
    const std::vector<std::string> &arg_value) {
  std::stringstream js;
  js << "angular.element('[ng-controller=" << controller << "]').scope().";
  AppendFunctionCall(func_name, arg_name, arg_value, &js);

  browser->GetMainFrame()->ExecuteJavaScript(js.str(), "", 0);
}


void JsExecutor::AppendFunctionCall(
    const std::string &func_name,
    const std::string &arg_name,
    const std::vector<std::string> &arg_value,
    std::ostream *out) {
  boost::property_tree::ptree arg_value_root;
  arg_value_root.add_child(arg_name, ToPtree(arg_value));

  *out << func_name << "(";
  boost::property_tree::write_json(*out, arg_value_root, false);
  *out << ")";
}


boost::property_tree::ptree
    JsExecutor::ToPtree(const std::vector<std::string> &values) {
  boost::property_tree::ptree arr;
  for (const std::string &value : values) {
    boost::property_tree::ptree node;
    node.put("", value);
    arr.push_back({"", node});
  }
  return std::move(arr);
}
}  // namespace ncstreamer
