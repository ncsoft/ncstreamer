/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#ifndef NCSTREAMER_CEF_SRC_STREAMING_SERVICE_TWITCH_CHAT_H_
#define NCSTREAMER_CEF_SRC_STREAMING_SERVICE_TWITCH_CHAT_H_


#include <deque>
#include <string>

#include "ncstreamer_cef/src/streaming_service/irc_service.h"


namespace ncstreamer {
class TwitchChat {
 public:
  TwitchChat();
  virtual ~TwitchChat();

  void Connect(
      const std::string &host,
      const std::string &port,
      const std::string &oauth,
      const std::string &user_id,
      const std::string &channel_name,
      IrcService::OnErrored on_errored);
  void Close();
  void ReadHandle(const std::string &msg);

 private:
  IrcService irc_;
  IrcService::OnErrored on_errored_;
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_STREAMING_SERVICE_TWITCH_CHAT_H_
