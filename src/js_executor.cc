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


template <typename T>
void JsExecutor::Execute(
    CefRefPtr<CefBrowser> browser,
    const std::string &func_name,
    const std::pair<std::string, std::string> &arg0,
    const std::pair<std::string, std::vector<T>> &arg1) {
  std::stringstream js;
  boost::property_tree::ptree args;

  args.add(arg0.first, arg0.second);
  args.add_child(arg1.first, ToPtree(arg1.second));

  AppendFunctionCall(func_name, args, &js);
  browser->GetMainFrame()->ExecuteJavaScript(js.str(), "", 0);
}


template
void JsExecutor::Execute<std::string>(
    CefRefPtr<CefBrowser> browser,
    const std::string &func_name,
    const std::pair<std::string, std::string> &arg0,
    const std::pair<std::string, std::vector<std::string>> &arg1);


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
  boost::property_tree::ptree args;
  args.add_child(arg_name, ToPtree(arg_value));

  AppendFunctionCall(func_name, args, out);
}


void JsExecutor::AppendFunctionCall(
    const std::string &func_name,
    const boost::property_tree::ptree &args,
    std::ostream *out) {
  *out << func_name << "(";
  boost::property_tree::write_json(*out, args, false);
  *out << ")";
}


template <typename T>
boost::property_tree::ptree
    JsExecutor::ToPtree(const std::vector<T> &values) {
  boost::property_tree::ptree arr;
  for (const T &value : values) {
    arr.push_back({"", BuildTree(value)});
  }
  return std::move(arr);
}


boost::property_tree::ptree JsExecutor::BuildTree(
    const std::string &value) {
  boost::property_tree::ptree node;
  node.put("", value);
  return node;
}
}  // namespace ncstreamer
