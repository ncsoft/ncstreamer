/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef SRC_STREAMING_SERVICE_FACEBOOK_H_
#define SRC_STREAMING_SERVICE_FACEBOOK_H_


#include "src/lib/cef_fit_client.h"
#include "src/lib/uri.h"
#include "src/streaming_service/streaming_service_provider.h"


namespace ncstreamer {
class Facebook : public StreamingServiceProvider {
 public:
  Facebook();
  virtual ~Facebook();

  void LogIn(
      HWND parent,
      const OnFailed &on_failed,
      const OnLoggedIn &on_logged_in) override;

 private:
  class FacebookClient;

  static const Uri kLoginRedirectUri;

  CefRefPtr<FacebookClient> facebook_client_;
};


class Facebook::FacebookClient : public CefFitClient {
 public:
  FacebookClient();
  virtual ~FacebookClient();

 private:
  IMPLEMENT_REFCOUNTING(FacebookClient);
};
}  // namespace ncstreamer


#endif  // SRC_STREAMING_SERVICE_FACEBOOK_H_
