/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/client/client_life_span_handler.h"

#include "include/cef_app.h"
#include "include/wrapper/cef_helpers.h"

#include "src/js_executor.h"
#include "src/lib/display.h"
#include "src/lib/window_frame_remover.h"
#include "src/manifest.h"
#include "src/resource.h"


namespace ncstreamer {
ClientLifeSpanHandler::ClientLifeSpanHandler(HINSTANCE instance)
    : icon_{::LoadIcon(instance, MAKEINTRESOURCE(IDI_NCSTREAMER_256PX))},
      browsers_{},
      main_browser_{} {
}


ClientLifeSpanHandler::~ClientLifeSpanHandler() {
}


void ClientLifeSpanHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  HWND wnd{browser->GetHost()->GetWindowHandle()};
  ::SendMessage(wnd, WM_SETICON, ICON_SMALL, (LPARAM) icon_);

  if (browser->IsPopup()) {
    JsExecutor::Execute(
        main_browser_, "onBeforePopupOpen", browser->GetIdentifier());
  }

  if (!main_browser_) {
    main_browser_ = browser;
    WindowFrameRemover::Get()->RegisterWindow(
        wnd, Display::Scale(kWindowTitlebarDragRect));
  }

  browsers_.emplace(browser->GetIdentifier(), browser);
}


bool ClientLifeSpanHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  return false;
}


void ClientLifeSpanHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  browsers_.erase(browser->GetIdentifier());

  if (browser->IsPopup()) {
    JsExecutor::Execute(
        main_browser_, "OnBeforePopupClose", browser->GetIdentifier());
  }

  if (browser->IsSame(main_browser_)) {
    for (auto elem : browsers_) {
      auto eachBrowser = elem.second;
      eachBrowser->GetHost()->CloseBrowser(true);
    }
  }

  if (browsers_.empty()) {
    HWND wnd = browser->GetHost()->GetWindowHandle();
    WindowFrameRemover::Get()->UnregisterWindow(wnd);
    main_browser_ = nullptr;
    ::CefQuitMessageLoop();
  }
}
}  // namespace ncstreamer
