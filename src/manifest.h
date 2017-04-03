/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_MANIFEST_H_
#define SRC_MANIFEST_H_


#include "src/lib/dimension.h"
#include "src/lib/windows_types.h"


namespace ncstreamer {
static const Dimension<int> kWindowMinimumSize{348 + 16, 603 - 11};
static const RECT kWindowTitlebarDragRect{5, 5, 270, 48};
}  // namespace ncstreamer


#endif  // SRC_MANIFEST_H_
