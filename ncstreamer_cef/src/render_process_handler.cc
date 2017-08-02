/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/render_process_handler.h"

#include "include/wrapper/cef_helpers.h"

#include "ncstreamer_cef/src/render_process/render_v8_handler.h"


namespace ncstreamer {
RenderProcessHandler::RenderProcessHandler()
    : load_handler_{new RenderLoadHandler{}} {
}


RenderProcessHandler::~RenderProcessHandler() {
}


CefRefPtr<CefLoadHandler> RenderProcessHandler::GetLoadHandler() {
  return load_handler_;
}


void RenderProcessHandler::OnContextCreated(CefRefPtr<CefBrowser> browser,
                                            CefRefPtr<CefFrame> frame,
                                            CefRefPtr<CefV8Context> context) {
  CEF_REQUIRE_RENDERER_THREAD();
  if (frame->IsValid() == false ||
      frame->IsMain() == false) {
    return;
  }
  CefRefPtr<CefV8Value> object = context->GetGlobal();
  CefRefPtr<CefV8Handler> handler = new RenderV8Handler();
  CefRefPtr<CefV8Value> func =
      CefV8Value::CreateFunction("sendEvent", handler);
  object->SetValue("sendEvent", func, V8_PROPERTY_ATTRIBUTE_NONE);
}
}  // namespace ncstreamer
