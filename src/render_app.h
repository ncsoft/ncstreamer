/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_RENDER_APP_H_
#define SRC_RENDER_APP_H_


#include "include/cef_app.h"


namespace ncstreamer {
class RenderApp : public CefApp {
 public:
  RenderApp();
  virtual ~RenderApp();

 private:
  IMPLEMENT_REFCOUNTING(RenderApp);
};
}  // namespace ncstreamer


#endif  // SRC_RENDER_APP_H_
