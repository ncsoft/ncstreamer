/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/lib/cef_types.h"


namespace ncstreamer {
void CefWindowRectangle::Reset(
    const Rectangle &rect,
    CefWindowInfo *window_info) {
  window_info->x = rect.x();
  window_info->y = rect.y();
  window_info->width = rect.width();
  window_info->height = rect.height();
}
}  // namespace ncstreamer
