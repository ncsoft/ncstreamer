/**
* Copyright (C) 2017 NCSOFT Corporation
*/


#include "ncstreamer_cef/src/streaming_service/twitch_chat.h"

#include <utility>

#include "boost/algorithm/string/regex.hpp"
#include "boost/bind.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/property_tree/ptree.hpp"


namespace ncstreamer {
TwitchChat::TwitchChat()
    : irc_{},
      on_errored_{},
      id_generated_{},
      reservoir_mutex_{},
      channel_name_{""} {
}


TwitchChat::~TwitchChat() {
}


void TwitchChat::Connect(
    const std::string &host,
    const std::string &port,
    const std::string &oauth,
    const std::string &nick_name,
    const std::string &channel_name,
    IrcService::OnErrored on_errored) {
  on_errored_ = on_errored;
  channel_name_ = channel_name;

  const std::string &msg = {
      "PASS oauth:" + oauth + "\r\n" +
      "NICK " + nick_name + "\r\n" +
      "JOIN #" + channel_name + "\r\n" +
      "CAP REQ :twitch.tv/membership\r\n" +
      "CAP REQ :twitch.tv/tags\r\n" +
      "CAP REQ :twitch.tv/commands\r\n"};

  // clear chats reservoir & index
  {
    std::lock_guard<std::mutex> lock{reservoir_mutex_};
    reservoir_.clear();
  }
  id_generated_ = 0;

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
  try {
    // chats reservoir 0:id, 1:time, 2:nick, 3:msg
    IrcMessage irc_msg(msg);
    if (irc_msg.IsUserChat()) {
      const std::string id = std::to_string(id_generated_++);
      const std::string time = boost::posix_time::to_simple_string(
          boost::posix_time::second_clock::local_time());
      const std::string sender = irc_msg.GetSender();
      const std::string content = irc_msg.GetContent(channel_name_);

      {
        std::lock_guard<std::mutex> lock{reservoir_mutex_};
        reservoir_.emplace_front(id, time, sender, content);

        // set max size to 20
        if (reservoir_.size() > 20)
          reservoir_.pop_back();
      }
    } else if (irc_msg.IsPingMessage()) {
      irc_.SendPongMessage();
    }
  } catch(...) {
    assert(false);
  }
}


const std::string TwitchChat::GetJson(const std::string &time) {
  std::ostringstream oss;
  try {
    using boost::property_tree::ptree;
    using boost::property_tree::json_parser::write_json;
    using boost::posix_time::ptime;
    using boost::posix_time::time_from_string;

    ptree root;
    ptree datas;
    {
      std::lock_guard<std::mutex> lock{reservoir_mutex_};

      for (const auto &chat : reservoir_) {
        const std::string ctime = std::get<1>(chat);

        ptime chat_time = time_from_string(ctime);
        if (time.length() > 4) {
          ptime user_time = time_from_string(time);
          if (chat_time < user_time)
            break;
        }

        const std::string id = std::get<0>(chat);
        const std::string nick = std::get<2>(chat);
        const std::string msg = std::get<3>(chat);

        ptree from;
        from.add<std::string>("name", nick);

        ptree data;
        data.add<std::string>("created_time", ctime);
        data.add_child("from", from);
        data.add<std::string>("message", msg);
        data.add<std::string>("id", id);

        datas.push_back(std::make_pair("", data));
      }
    }
    root.add_child("data", datas);

    write_json(oss, root);
  } catch (...) {
    assert(false);
  }
  return oss.str();
}


IrcService::ReadyStatus TwitchChat::GetReadyStatus() {
  return irc_.GetReadyStatus();
}


TwitchChat::IrcMessage::IrcMessage(const std::string &msg)
    : msg_{msg} {
  Tokenize(msg);
}


bool TwitchChat::IrcMessage::IsUserChat() {
  irctypes::separator sep(" ");

  for (const auto &token : tokens_) {
    irctypes::tokenizer tok(token, sep);
    for (const auto &elem : tok) {
      if (elem == "PRIVMSG")
        return true;
    }
  }
  return false;
}


bool TwitchChat::IrcMessage::IsPingMessage() {
    if (msg_ == "PING :tmi.twitch.tv")
      return true;

  return false;
}


const std::string TwitchChat::IrcMessage::GetSender() {
  std::string::size_type pos{0};
  std::string::size_type found{0};
  std::string::size_type length{0};
  const std::string &iden{"="};

  for (const auto &token : tokens_) {
    length = token.length();

    pos = token.find_first_not_of(iden);
    if (pos != std::string::npos) {
      found = token.find_first_of(iden);
      if (found != std::string::npos) {
        std::string tok = token.substr(pos, found - pos);
        if ((tok == "display-name") && ((found + 1) != length)) {
          return token.substr((found + 1), length);
        }
      }
    }
  }
  return "";
}


const std::string TwitchChat::IrcMessage::GetContent(
    const std::string &channel_name) {
  irctypes::separator sep{" "};

  for (const auto &token : tokens_) {
    irctypes::tokenizer tok{token, sep};
    for (const auto &elem : tok) {
      if (elem == "PRIVMSG") {
        const std::string &iden = {"#" + channel_name + " :"};
        std::vector<std::string> outs;

        // split whole message
        boost::algorithm::split_regex(outs, msg_, boost::regex{iden});
        if (outs.size() > 1)
          return outs[1];
      }
    }
  }
  return "";
}


void TwitchChat::IrcMessage::Tokenize(const std::string &msg) {
  irctypes::separator sep(";");
  irctypes::tokenizer tokens(msg, sep);

  tokens_.clear();
  for (const auto &token : tokens) {
    tokens_.emplace_back(token);
  }
}
}  // namespace ncstreamer
