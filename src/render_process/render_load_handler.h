/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_RENDER_PROCESS_RENDER_LOAD_HANDLER_H_
#define SRC_RENDER_PROCESS_RENDER_LOAD_HANDLER_H_


#include "include/cef_load_handler.h"


namespace ncstreamer {
class RenderLoadHandler : public CefLoadHandler {
 public:
  RenderLoadHandler();
  virtual ~RenderLoadHandler();

 private:
  IMPLEMENT_REFCOUNTING(RenderLoadHandler);
};
}  // namespace ncstreamer


#endif  // SRC_RENDER_PROCESS_RENDER_LOAD_HANDLER_H_
