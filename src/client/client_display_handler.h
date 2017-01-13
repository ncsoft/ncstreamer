/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_CLIENT_CLIENT_DISPLAY_HANDLER_H_
#define SRC_CLIENT_CLIENT_DISPLAY_HANDLER_H_


#include "include/cef_display_handler.h"


namespace ncstreamer {
class ClientDisplayHandler : public CefDisplayHandler {
 public:
  ClientDisplayHandler();
  virtual ~ClientDisplayHandler();

  void OnTitleChange(CefRefPtr<CefBrowser> browser,
                     const CefString &title) override;

 private:
  IMPLEMENT_REFCOUNTING(ClientDisplayHandler);
};
}  // namespace ncstreamer


#endif  // SRC_CLIENT_CLIENT_DISPLAY_HANDLER_H_
