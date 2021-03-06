/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_cef/src/lib/cef_fit_client.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <unordered_map>

#include "include/base/cef_bind.h"
#include "include/base/cef_logging.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

#include "ncstreamer_cef/src/render_process_message_types.h"


namespace ncstreamer {
CefFitClient::CefFitClient() {
}


CefFitClient::~CefFitClient() {
}


bool CefFitClient::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) {
  CEF_REQUIRE_UI_THREAD();

  if (!message->IsValid()) {
    return false;
  }

  switch (source_process) {
    case PID_RENDERER: {
      return OnRenderProcessMessageReceived(browser, message);
    }
    default: {
      DCHECK(false);
      return false;
    }
  }
}


int CefFitClient::GetIncCap(int current, int preferable, std::size_t index) {
  int gap = preferable - current;
  if (gap == 0) {
    return 0;
  }

  unsigned int u_gap = std::abs(gap);
  static const double kBase{1.2};
  unsigned int inc = std::lround(std::pow(kBase, index));
  unsigned int next_inc = std::lround(std::pow(kBase, index + 1));
  if (inc + next_inc > u_gap) {
    return gap;
  }
  return (gap > 0) ? inc : 0 - inc;
}


bool CefFitClient::OnRenderProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefProcessMessage> message) {
  CEF_REQUIRE_UI_THREAD();

  CefString msg_name = message->GetName();
  if (msg_name == RenderProcessMessage::kScrollGap) {
    return OnRenderProcessScrollGap(browser, message);
  }

  DCHECK(false);
  return false;
}


bool CefFitClient::OnRenderProcessScrollGap(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefProcessMessage> message) {
  CEF_REQUIRE_UI_THREAD();

  auto args = message->GetArgumentList();
  int gap_w = args->GetInt(0);
  int gap_h = args->GetInt(1);

  ResizeBrowser(browser, {gap_w, gap_h});
  return true;
}


bool CefFitClient::ResizeBrowser(
    CefRefPtr<CefBrowser> browser,
    const Dimension<int> &inc) {
  CEF_REQUIRE_UI_THREAD();

  if (inc.empty()) {
    return true;
  }

  HWND wnd = browser->GetHost()->GetWindowHandle();
  RECT rect;
  ::GetWindowRect(wnd, &rect);
  LONG rect_w = rect.right - rect.left;
  LONG rect_h = rect.bottom - rect.top;

  static const int kMaxWidth = ::GetSystemMetrics(SM_CXSCREEN);
  static const int kMaxHeight = ::GetSystemMetrics(SM_CYSCREEN);

  int w_inc = std::min<int>(inc.width(), kMaxWidth - rect_w);
  int h_inc = std::min<int>(inc.height(), kMaxHeight - rect_h);

  Rectangle preferable{
      rect.left - w_inc / 2,
      rect.top - h_inc / 2,
      rect_w + w_inc,
      rect_h + h_inc};

  ResizeBrowserSmoothly(browser, preferable, 0);
  return true;
}


void CefFitClient::ResizeBrowserSmoothly(
    CefRefPtr<CefBrowser> browser,
    const Rectangle &preferable,
    std::size_t index) {
  HWND wnd = browser->GetHost()->GetWindowHandle();
  RECT current;
  ::GetWindowRect(wnd, &current);
  LONG current_w = current.right - current.left;
  LONG current_h = current.bottom - current.top;

  if (current_w == preferable.width() &&
      current_h == preferable.height()) {
    return;
  }

  int next_x = current.left;
  int next_y = current.top;
  int next_w = current_w;
  int next_h = current_h;
  if (current_w != preferable.width()) {
    int inc_w = GetIncCap(current_w, preferable.width(), index);
    next_x = current.left - (inc_w / 2);
    next_w = current_w + inc_w;
  }
  if (current_h != preferable.height()) {
    int inc_h = GetIncCap(current_h, preferable.height(), index);
    next_y = current.top - (inc_h / 2);
    next_h = current_h + inc_h;
  }

  BOOL result = ::MoveWindow(wnd, next_x, next_y, next_w, next_h, TRUE);
  if (result == FALSE) {
    return;
  }

  browser->GetHost()->NotifyMoveOrResizeStarted();

  ::CefPostDelayedTask(
      TID_UI,
      base::Bind(&CefFitClient::ResizeBrowserSmoothly, this,
          browser, preferable, index + 1),
      10 /*millisec*/);
}
}  // namespace ncstreamer
