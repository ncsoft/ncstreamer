/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_RENDER_APP_H_
#define NCSTREAMER_CEF_SRC_RENDER_APP_H_


#include "include/cef_app.h"

#include "ncstreamer_cef/src/render_process_handler.h"


namespace ncstreamer {
class RenderApp : public CefApp {
 public:
  RenderApp();
  virtual ~RenderApp();

  CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override;

 private:
  CefRefPtr<RenderProcessHandler> render_process_handler_;

  IMPLEMENT_REFCOUNTING(RenderApp);
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_RENDER_APP_H_
