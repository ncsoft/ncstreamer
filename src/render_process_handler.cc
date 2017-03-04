/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/render_process_handler.h"


namespace ncstreamer {
RenderProcessHandler::RenderProcessHandler()
    : load_handler_{new RenderLoadHandler{}} {
}


RenderProcessHandler::~RenderProcessHandler() {
}


CefRefPtr<CefLoadHandler> RenderProcessHandler::GetLoadHandler() {
  return load_handler_;
}
}  // namespace ncstreamer
