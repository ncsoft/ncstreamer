/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/lib/window_frame_remover.h"

#include <cassert>

#include "dwmapi.h"  // NOLINT
#include "windowsx.h"  // NOLINT


namespace ncstreamer {
void WindowFrameRemover::SetUp() {
  assert(!static_instance);
  static_instance = new WindowFrameRemover{};
}


void WindowFrameRemover::ShutDown() {
  assert(static_instance);
  delete static_instance;
  static_instance = nullptr;
}


WindowFrameRemover *WindowFrameRemover::Get() {
  assert(static_instance);
  return static_instance;
}


void WindowFrameRemover::RegisterWindow(
    HWND wnd,
    const RECT &drag_client_rect) {
  windows_.emplace(wnd, Window{wnd, drag_client_rect});
}


void WindowFrameRemover::UnregisterWindow(HWND wnd) {
  windows_.erase(wnd);
}


WindowFrameRemover::Window::Window(
    HWND wnd,
    const RECT &drag_client_rect)
    : window_{wnd},
      drag_client_rect_{drag_client_rect},
      prev_mouse_point_{0, 0} {
  DwmDropShadow();

  ::SetWindowLong(wnd, GWL_STYLE, NULL);
  ::SetWindowPos(wnd, HWND_TOP, 0, 0, 0, 0,
      SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}


WindowFrameRemover::Window::~Window() {
}


LRESULT WindowFrameRemover::Window::OnMouseHook(
    int code, WPARAM wparam, LPARAM lparam) {
  const DWORD &event_type = (DWORD) wparam;
  const MOUSEHOOKSTRUCT *hook_info =
      reinterpret_cast<MOUSEHOOKSTRUCT *>(lparam);
  const POINT &pt = hook_info->pt;

  switch (event_type) {
    case WM_LBUTTONDOWN: {
      RECT wnd_rect;
      ::GetWindowRect(window_, &wnd_rect);

      RECT drag_wnd_rect{
          wnd_rect.left + drag_client_rect_.left,
          wnd_rect.top + drag_client_rect_.top,
          wnd_rect.left + drag_client_rect_.right,
          wnd_rect.top + drag_client_rect_.bottom};
      if (::PtInRect(&drag_wnd_rect, pt) == TRUE) {
        ::SetCapture(window_);
        prev_mouse_point_ = pt;
        return NULL;
      }
      break;
    }
    case WM_MOUSEMOVE: {
      if (::GetCapture() == window_) {
        RECT wnd_rect;
        ::GetWindowRect(window_, &wnd_rect);
        int width{wnd_rect.right - wnd_rect.left};
        int height{wnd_rect.bottom - wnd_rect.top};

        POINT delta{pt.x - prev_mouse_point_.x,
                    pt.y - prev_mouse_point_.y};

        if (delta.x != 0 || delta.y != 0) {
          ::MoveWindow(window_,
                       wnd_rect.left + delta.x,
                       wnd_rect.top + delta.y,
                       width,
                       height,
                       TRUE);

          prev_mouse_point_ = pt;
          return NULL;
        }
      }
      break;
    }
    case WM_LBUTTONUP: {
      if (::GetCapture() == window_) {
        ::ReleaseCapture();
        return NULL;
      }
      break;
    }
    default: {
      break;
    }
  }

  return ::CallNextHookEx(NULL, code, wparam, lparam);
}


HRESULT WindowFrameRemover::Window::DwmDropShadow() {
  int policy{DWMNCRP_ENABLED};
  ::DwmSetWindowAttribute(
      window_, DWMWA_NCRENDERING_POLICY, &policy, sizeof(policy));

  const MARGINS kMargins{-1, -1, -1, -1};
  return ::DwmExtendFrameIntoClientArea(window_, &kMargins);
}


WindowFrameRemover::WindowFrameRemover()
    : mouse_hook_{::SetWindowsHookEx(
          WH_MOUSE, OnMouseHook, NULL, ::GetCurrentThreadId())},
      windows_{} {
}


WindowFrameRemover::~WindowFrameRemover() {
  ::UnhookWindowsHookEx(mouse_hook_);
}


LRESULT CALLBACK WindowFrameRemover::OnMouseHook(
    int code, WPARAM wparam, LPARAM lparam) {
  return Get()->OnThisMouseHook(code, wparam, lparam);
}


LRESULT WindowFrameRemover::OnThisMouseHook(
    int code, WPARAM wparam, LPARAM lparam) {
  const MOUSEHOOKSTRUCT *hook_info =
      reinterpret_cast<MOUSEHOOKSTRUCT *>(lparam);
  HWND wnd = ::GetAncestor(hook_info->hwnd, GA_ROOT);

  auto i = windows_.find(wnd);
  if (i == windows_.end()) {
    return ::CallNextHookEx(NULL, code, wparam, lparam);
  }
  auto *window = &(i->second);
  return window->OnMouseHook(code, wparam, lparam);
}


WindowFrameRemover *WindowFrameRemover::static_instance{nullptr};
}  // namespace ncstreamer
