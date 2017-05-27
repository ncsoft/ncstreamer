/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/js_executor.h"

#include <sstream>
#include <utility>

#include "boost/property_tree/json_parser.hpp"

#include "ncstreamer_cef/src/lib/string.h"


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
    const std::string &arg_value) {
  std::stringstream js;
  js << func_name << "('" << String::EscapeSingleQuote(arg_value) << "')";

  browser->GetMainFrame()->ExecuteJavaScript(js.str(), "", 0);
}


void JsExecutor::Execute(
    CefRefPtr<CefBrowser> browser,
    const std::string &func_name,
    const boost::property_tree::ptree &arg_value) {
  std::stringstream js;
  AppendFunctionCall(func_name, arg_value, &js);

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


void JsExecutor::Execute(
    CefRefPtr<CefBrowser> browser,
    const std::string &func_name,
    const std::string &arg0,
    const std::pair<std::string, std::string> &arg1_0) {
  std::stringstream js;
  boost::property_tree::ptree arg1;

  arg1.add(arg1_0.first, arg1_0.second);

  AppendFunctionCall(func_name, arg0, arg1, &js);
  browser->GetMainFrame()->ExecuteJavaScript(js.str(), "", 0);
}


void JsExecutor::Execute(
    CefRefPtr<CefBrowser> browser,
    const std::string &func_name,
    const std::string &arg0,
    const boost::property_tree::ptree &arg1) {
  std::stringstream js;
  AppendFunctionCall(func_name, arg0, arg1, &js);

  browser->GetMainFrame()->ExecuteJavaScript(js.str(), "", 0);
}


void JsExecutor::Execute(
    CefRefPtr<CefBrowser> browser,
    const std::string &func_name,
    const int &arg0,
    const boost::property_tree::ptree &arg1) {
  std::stringstream js;
  AppendFunctionCall(func_name, arg0, arg1, &js);

  browser->GetMainFrame()->ExecuteJavaScript(js.str(), "", 0);
}


template <typename T>
void JsExecutor::Execute(
    CefRefPtr<CefBrowser> browser,
    const std::string &func_name,
    const std::string &arg0,
    const std::pair<std::string, std::string> &arg1_0,
    const std::pair<std::string, std::vector<T>> &arg1_1) {
  std::stringstream js;
  boost::property_tree::ptree arg1;

  arg1.add(arg1_0.first, arg1_0.second);
  arg1.add_child(arg1_1.first, ToPtree(arg1_1.second));

  AppendFunctionCall(func_name, arg0, arg1, &js);
  browser->GetMainFrame()->ExecuteJavaScript(js.str(), "", 0);
}


template
void JsExecutor::Execute<std::string>(
    CefRefPtr<CefBrowser> browser,
    const std::string &func_name,
    const std::string &arg0,
    const std::pair<std::string, std::string> &arg1_0,
    const std::pair<std::string, std::vector<std::string>> &arg1_1);
template
void JsExecutor::Execute<std::string>(
    CefRefPtr<CefBrowser> browser,
    const std::string &func_name,
    const std::string &arg0,
    const std::pair<std::string, std::string> &arg1_1,
    const std::pair<std::string, std::vector<std::string>> &arg1_2);
template
void JsExecutor::Execute<boost::property_tree::ptree>(
    CefRefPtr<CefBrowser> browser,
    const std::string &func_name,
    const std::string &arg0,
    const std::pair<std::string, std::string> &arg1_0,
    const std::pair<std::string,
                    std::vector<boost::property_tree::ptree>> &arg1_1);


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


template <typename T>
boost::property_tree::ptree
    JsExecutor::ToPtree(const std::vector<T> &values) {
  boost::property_tree::ptree arr;
  for (const T &value : values) {
    arr.push_back({"", BuildTree(value)});
  }
  return std::move(arr);
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
    const boost::property_tree::ptree &arg,
    std::ostream *out) {
  *out << func_name << "(";
  boost::property_tree::write_json(*out, arg, false);
  *out << ")";
}


void JsExecutor::AppendFunctionCall(
    const std::string &func_name,
    const std::string &arg0,
    const boost::property_tree::ptree &arg1,
    std::ostream *out) {
  *out << func_name << "(";
  *out << "'" << arg0 << "',";
  boost::property_tree::write_json(*out, arg1, false);
  *out << ")";
}


void JsExecutor::AppendFunctionCall(
    const std::string &func_name,
    const int &arg0,
    const boost::property_tree::ptree &arg1,
    std::ostream *out) {
  *out << func_name << "(";
  *out << arg0 << ",";
  boost::property_tree::write_json(*out, arg1, false);
  *out << ")";
}


boost::property_tree::ptree JsExecutor::BuildTree(
    const std::string &value) {
  boost::property_tree::ptree node;
  node.put("", value);
  return node;
}


boost::property_tree::ptree JsExecutor::BuildTree(
    const boost::property_tree::ptree &value) {
  return value;
}
}  // namespace ncstreamer
