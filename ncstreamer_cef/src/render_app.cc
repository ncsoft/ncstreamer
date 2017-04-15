/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/render_app.h"


namespace ncstreamer {
RenderApp::RenderApp()
    : render_process_handler_{new RenderProcessHandler{}} {
}


RenderApp::~RenderApp() {
}


CefRefPtr<CefRenderProcessHandler> RenderApp::GetRenderProcessHandler() {
  return render_process_handler_;
}
}  // namespace ncstreamer
