/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#ifndef NCSTREAMER_CEF_SRC_STREAMING_SERVICE_YOUTUBE_APP_SECRET_H_
#define NCSTREAMER_CEF_SRC_STREAMING_SERVICE_YOUTUBE_APP_SECRET_H_


namespace ncstreamer {
static const char *kYouTubeAppSecret{[]() {
  static const char *kProduction{
      "<YouTube App Secret>"};  // Forbidden for security.
  return kProduction;
}()};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_STREAMING_SERVICE_YOUTUBE_APP_SECRET_H_
