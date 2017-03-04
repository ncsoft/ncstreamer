/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/client.h"

#include <algorithm>
#include <functional>
#include <unordered_map>

#include "include/base/cef_logging.h"
#include "include/wrapper/cef_helpers.h"

#include "src/render_process_message_types.h"


namespace ncstreamer {
Client::Client(HINSTANCE instance)
    : display_handler_{new ClientDisplayHandler{}},
      life_span_handler_{new ClientLifeSpanHandler{instance}},
      load_handler_{new ClientLoadHandler{}},
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


bool Client::OnRenderProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefProcessMessage> message) {
  CEF_REQUIRE_UI_THREAD();

  using RenderProcessMessageHandler =
      std::function<bool(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefProcessMessage> message)>;
  static const std::unordered_map<std::wstring, RenderProcessMessageHandler>
      kRenderProcessMessageHandlers{
          {RenderProcessMessage::kScrollGap, OnRenderProcessScrollGap}};

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
    Dimension inc) {
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

  int x = rect.left - w_inc / 2;
  int y = rect.top - h_inc / 2;
  int width = rect_w + w_inc;
  int height = rect_h + h_inc;

  BOOL result = ::MoveWindow(wnd, x, y, width, height, TRUE);
  if (result == FALSE) {
    return false;
  }

  browser->GetHost()->NotifyMoveOrResizeStarted();
  return true;
}
}  // namespace ncstreamer
