/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_LIB_WINDOW_FRAME_REMOVER_H_
#define NCSTREAMER_CEF_SRC_LIB_WINDOW_FRAME_REMOVER_H_


#include <unordered_map>

#include "windows.h"  // NOLINT


namespace ncstreamer {
class WindowFrameRemover {
 public:
  static const DWORD kWindowStyleBeforeInitialization{NULL};

  static void SetUp();
  static void ShutDown();
  static WindowFrameRemover *Get();

  void RegisterWindow(
      HWND wnd,
      const RECT &drag_client_rect);
  void UnregisterWindow(HWND wnd);

 private:
  class Window {
   public:
    Window(
        HWND wnd,
        const RECT &drag_client_rect);
    virtual ~Window();

    LRESULT OnMouseHook(
        int code, WPARAM wparam, LPARAM lparam);

   private:
    using LpWndProc = LRESULT (CALLBACK *)(HWND, UINT, WPARAM, LPARAM);

    HRESULT DwmDropShadow();

    const HWND window_;
    const RECT drag_client_rect_;

    POINT prev_mouse_point_;
  };

  using WindowMap = std::unordered_map<HWND, Window>;

  WindowFrameRemover();
  virtual ~WindowFrameRemover();

  static LRESULT CALLBACK OnMouseHook(
      int code, WPARAM wparam, LPARAM lparam);

  LRESULT OnThisMouseHook(
      int code, WPARAM wparam, LPARAM lparam);

  static WindowFrameRemover *static_instance;

  HHOOK mouse_hook_;
  WindowMap windows_;
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_LIB_WINDOW_FRAME_REMOVER_H_
