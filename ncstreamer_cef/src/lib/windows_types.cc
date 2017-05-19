/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/lib/windows_types.h"

#include "Shlobj.h"  // NOLINT


namespace ncstreamer {
Rectangle Windows::GetWindowRectangle(HWND wnd) {
  RECT r;
  ::GetWindowRect(wnd, &r);

  return Rectangle{
      r.left,
      r.top,
      r.right - r.left,
      r.bottom - r.top};
}


std::wstring Windows::GetUserLocalAppDataPath() {
  wchar_t *task_mem;
  HRESULT result = ::SHGetKnownFolderPath(
      FOLDERID_LocalAppData,  // %LOCALAPPDATA% (%USERPROFILE%\AppData\Local)
      KF_FLAG_DEFAULT,  // current path
      NULL,  // current user
      &task_mem);

  if (result != S_OK) {
    return std::wstring{};
  }

  std::wstring path{task_mem};
  ::CoTaskMemFree(task_mem);

  return path;
}
}  // namespace ncstreamer
