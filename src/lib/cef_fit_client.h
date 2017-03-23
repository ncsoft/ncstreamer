/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_LIB_CEF_FIT_CLIENT_H_
#define SRC_LIB_CEF_FIT_CLIENT_H_


#include "include/cef_client.h"

#include "src/lib/dimension.h"
#include "src/lib/rectangle.h"


namespace ncstreamer {
class CefFitClient : public CefClient {
 public:
  CefFitClient();
  virtual ~CefFitClient();

  bool OnProcessMessageReceived(
      CefRefPtr<CefBrowser> browser,
      CefProcessId source_process,
      CefRefPtr<CefProcessMessage> message) override;

 private:
  static int GetIncCap(int current, int preferable, std::size_t index);

  bool OnRenderProcessMessageReceived(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefProcessMessage> message);

  bool OnRenderProcessScrollGap(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefProcessMessage> message);

  bool ResizeBrowser(
      CefRefPtr<CefBrowser> browser,
      const Dimension<int> &inc);

  void ResizeBrowserSmoothly(
      CefRefPtr<CefBrowser> browser,
      const Rectangle &preferable,
      std::size_t index);

  IMPLEMENT_REFCOUNTING(CefFitClient);
};
}  // namespace ncstreamer


#endif  // SRC_LIB_CEF_FIT_CLIENT_H_
