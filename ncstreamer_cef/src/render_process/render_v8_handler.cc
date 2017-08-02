/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#include "ncstreamer_cef/src/render_process/render_v8_handler.h"

#include "include/wrapper/cef_helpers.h"

#include "ncstreamer_cef/src/render_process_message_types.h"


namespace ncstreamer {
RenderV8Handler::RenderV8Handler() {
}


RenderV8Handler::~RenderV8Handler() {
}


bool RenderV8Handler::Execute(const CefString &name,
                              CefRefPtr<CefV8Value> object,
                              const CefV8ValueList &arguments,
                              CefRefPtr<CefV8Value> &retval,
                              CefString &exception) {
  CEF_REQUIRE_RENDERER_THREAD();
  if (name != "sendEvent") {
    return false;
  }

  static const CefString kMsgType{RenderProcessMessage::kEvent};
  CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(kMsgType);
  CefRefPtr<CefListValue> args = msg->GetArgumentList();
  for (std::size_t i = 0; i < arguments.size(); ++i) {
    args->SetString(i, arguments.at(i)->GetStringValue());
  }

  CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
  CefRefPtr<CefBrowser> browser = context->GetBrowser();
  browser->SendProcessMessage(PID_BROWSER, msg);
  return true;
}
}  // namespace ncstreamer
