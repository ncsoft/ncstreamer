/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#ifndef NCSTREAMER_CEF_SRC_RENDER_PROCESS_RENDER_V8_HANDLER_H_
#define NCSTREAMER_CEF_SRC_RENDER_PROCESS_RENDER_V8_HANDLER_H_


#include "include/cef_v8.h"


namespace ncstreamer {
class RenderV8Handler : public CefV8Handler {
 public:
  RenderV8Handler();
  virtual ~RenderV8Handler();

  bool Execute(const CefString &name,
               CefRefPtr<CefV8Value> object,
               const CefV8ValueList &arguments,
               CefRefPtr<CefV8Value> &retval,
               CefString &exception) override;

 private:
  IMPLEMENT_REFCOUNTING(RenderV8Handler);
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_RENDER_PROCESS_RENDER_V8_HANDLER_H_
