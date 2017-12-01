/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#ifndef NCSTREAMER_CEF_SRC_STREAMING_SERVICE_TWITCH_CHAT_H_
#define NCSTREAMER_CEF_SRC_STREAMING_SERVICE_TWITCH_CHAT_H_


#include <deque>
#include <mutex>  // NOLINT
#include <string>
#include <tuple>
#include <vector>

#include "boost/tokenizer.hpp"

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
      const std::string &nick_name,
      const std::string &channel_name,
      IrcService::OnErrored on_errored);
  void Close();

  void ReadHandle(const std::string &msg);
  const std::string GetJson(const std::string &time);

  IrcService::ReadyType GetReady();

 private:
  class IrcMessage;

  IrcService irc_;
  IrcService::OnErrored on_errored_;

  // chats reservoir 0:id, 1:time, 2:nick, 3:msg
  mutable std::mutex reservoir_mutex_;
  std::deque<std::tuple<std::string, std::string, std::string,
      std::string>> reservoir_;
  unsigned int id_generated_;
};


class TwitchChat::IrcMessage {
 public:
  explicit IrcMessage(const std::string &msg);

  bool IsUserChat();
  const std::string GetSender();
  const std::string GetContent();

 private:
  void Tokenize(const std::string &msg);

  std::vector<std::string> tokens_;
};
}  // namespace ncstreamer


namespace irctypes {
  typedef boost::char_separator<char> separator;
  typedef boost::tokenizer<separator> tokenizer;
}  // namespace irctypes


#endif  // NCSTREAMER_CEF_SRC_STREAMING_SERVICE_TWITCH_CHAT_H_
