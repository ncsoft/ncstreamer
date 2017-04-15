/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_MANIFEST_H_
#define NCSTREAMER_CEF_SRC_MANIFEST_H_


#include "ncstreamer_cef/src/lib/dimension.h"
#include "ncstreamer_cef/src/lib/windows_types.h"


namespace ncstreamer {
static const Dimension<int> kWindowMinimumSize{348 + 16, 603 - 11};
static const RECT kWindowTitlebarDragRect{5, 5, 270, 48};

static const wchar_t *kDefaultUiUri{L"file:///ui/app.html"};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_MANIFEST_H_
