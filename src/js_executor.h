/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_JS_EXECUTOR_H_
#define SRC_JS_EXECUTOR_H_


#include <string>
#include <vector>

#include "include/cef_browser.h"


namespace ncstreamer {
class JsExecutor {
 public:
  static void ExecuteAngularJs(CefRefPtr<CefBrowser> browser,
                               const std::string &controller,
                               const std::string &func_name,
                               const std::vector<std::string> &args);
};
}  // namespace ncstreamer


#endif  // SRC_JS_EXECUTOR_H_
