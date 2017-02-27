/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_JS_EXECUTOR_H_
#define SRC_JS_EXECUTOR_H_


#include <ostream>
#include <string>
#include <vector>

#include "include/cef_browser.h"


namespace ncstreamer {
class JsExecutor {
 public:
  static void Execute(CefRefPtr<CefBrowser> browser,
                      const std::string &func_name);
  static void Execute(CefRefPtr<CefBrowser> browser,
                      const std::string &func_name,
                      const std::string &arg_name,
                      const std::vector<std::string> &arg_value);
  static void ExecuteAngularJs(CefRefPtr<CefBrowser> browser,
                               const std::string &controller,
                               const std::string &func_name,
                               const std::string &arg_name,
                               const std::vector<std::string> &arg_value);
 private:
  static void AppendFunctionCall(const std::string &func_name,
                                 const std::string &arg_name,
                                 const std::vector<std::string> &arg_value,
                                 std::ostream *out);
};
}  // namespace ncstreamer


#endif  // SRC_JS_EXECUTOR_H_
