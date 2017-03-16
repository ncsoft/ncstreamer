/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/client.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <unordered_map>

#include "include/base/cef_bind.h"
#include "include/base/cef_logging.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

#include "src/render_process_message_types.h"


namespace ncstreamer {
Client::Client(HINSTANCE instance,
               bool needs_to_find_sources,
               const std::vector<std::string> &sources)
    : display_handler_{new ClientDisplayHandler{}},
      life_span_handler_{new ClientLifeSpanHandler{instance}},
      load_handler_{new ClientLoadHandler{needs_to_find_sources,
                                          sources}},
      request_handler_{new ClientRequestHandler{}} {
}


Client::~Client() {
}


CefRefPtr<CefDisplayHandler> Client::GetDisplayHandler() {
  return display_handler_;
}


CefRefPtr<CefLifeSpanHandler> Client::GetLifeSpanHandler() {
  return life_span_handler_;
}


CefRefPtr<CefLoadHandler> Client::GetLoadHandler() {
  return load_handler_;
}


CefRefPtr<CefRequestHandler> Client::GetRequestHandler() {
  return request_handler_;
}


bool Client::OnProcessMessageReceived(
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


int Client::GetIncCap(int current, int preferable, std::size_t index) {
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


bool Client::OnRenderProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefProcessMessage> message) {
  CEF_REQUIRE_UI_THREAD();

  using RenderProcessMessageHandler =
      std::function<bool(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefProcessMessage> message)>;
  static const std::unordered_map<std::wstring, RenderProcessMessageHandler>
      kRenderProcessMessageHandlers{
          {RenderProcessMessage::kScrollGap,
           std::bind(&Client::OnRenderProcessScrollGap, this,
               std::placeholders::_1, std::placeholders::_2)}};

  CefString msg_name = message->GetName();
  auto i = kRenderProcessMessageHandlers.find(msg_name);
  if (i == kRenderProcessMessageHandlers.end()) {
    DCHECK(false);
    return false;
  }
  return i->second(browser, message);
}


bool Client::OnRenderProcessScrollGap(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefProcessMessage> message) {
  CEF_REQUIRE_UI_THREAD();

  auto args = message->GetArgumentList();
  int gap_w = args->GetInt(0);
  int gap_h = args->GetInt(1);

  ResizeBrowser(browser, {gap_w, gap_h});
  return true;
}


bool Client::ResizeBrowser(
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


void Client::ResizeBrowserSmoothly(
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
      base::Bind(&Client::ResizeBrowserSmoothly, this,
          browser, preferable, index + 1),
      10 /*millisec*/);
}
}  // namespace ncstreamer
