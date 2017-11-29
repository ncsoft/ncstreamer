/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#include "ncstreamer_cef/src/streaming_service/twitch_chat.h"

#include "boost/bind.hpp"


namespace ncstreamer {
TwitchChat::TwitchChat()
    : irc_{},
      on_errored_{} {
}


TwitchChat::~TwitchChat() {
}


void TwitchChat::Connect(
    const std::string &host,
    const std::string &port,
    const std::string &oauth,
    const std::string &user_id,
    const std::string &channel_name,
    IrcService::OnErrored on_errored) {
  on_errored_ = on_errored;

  const std::string &msg = {
      "PASS oauth:" + oauth + "\r\n" +
      "NICK " + user_id + "\r\n" +
      "JOIN #" + channel_name + "\r\n" +
      "CAP REQ :twitch.tv/membership\r\n" +
      "CAP REQ :twitch.tv/tags\r\n" +
      "CAP REQ :twitch.tv/commands\r\n"};

  irc_.Connect(host, port, msg,
      [this](const boost::system::error_code &ec) {
    on_errored_(ec);
  }, [this](const std::string &msg) {
    ReadHandle(msg);
  });
}


void TwitchChat::Close() {
  irc_.Close();
}


void TwitchChat::ReadHandle(const std::string &msg) {
  // store in reservoir
}
}  // namespace ncstreamer
