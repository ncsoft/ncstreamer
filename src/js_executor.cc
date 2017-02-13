/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/js_executor.h"

#include <sstream>


namespace ncstreamer {
void JsExecutor::ExecuteAngularJs(
    CefRefPtr<CefBrowser> browser,
    const std::string &controller,
    const std::string &func_name,
    const std::vector<std::string> &args) {
  std::stringstream js;
  js << "angular.element('[ng-controller=" << controller << "]').scope().";
  js << func_name << "(";

  if (!args.empty()) {
    auto i = args.begin();
    js << "'" << *i << "'";
    for (++i; i != args.end(); ++i) {
      js << ", ";
      js << "'" << *i << "'";
    }
  }

  js << ")";

  browser->GetMainFrame()->ExecuteJavaScript(js.str(), "", 0);
}
}  // namespace ncstreamer
