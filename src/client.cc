/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "src/client.h"


namespace ncstreamer {
Client::Client(
    HINSTANCE instance,
    bool shows_sources_all,
    const std::vector<std::string> &sources,
    const std::wstring &locale)
    : display_handler_{new ClientDisplayHandler{}},
      life_span_handler_{new ClientLifeSpanHandler{instance}},
      load_handler_{new ClientLoadHandler{shows_sources_all,
                                          sources}},
      request_handler_{new ClientRequestHandler{locale}} {
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
}  // namespace ncstreamer
