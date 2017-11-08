/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_REMOTE_MESSAGE_TYPES_H_
#define NCSTREAMER_CEF_SRC_REMOTE_MESSAGE_TYPES_H_


namespace ncstreamer {
class RemoteMessage {
 public:
  enum class MessageType {
    kUndefined = 0,
    kStreamingStatusRequest = 101,
    kStreamingStatusResponse,
    kStreamingStartRequest = 201,
    kStreamingStartResponse,
    kStreamingStartEvent,
    kStreamingStopRequest = 211,
    kStreamingStopResponse,
    kStreamingStopEvent,
    kSettingsQualityUpdateRequest = 301,
    kSettingsQualityUpdateResponse,
    kStreamingCommentsRequest = 401,
    kStreamingCommentsResponse,
    kNcStreamerExitRequest = 901,
    kNcStreamerExitResponse,  // not used.
  };
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_REMOTE_MESSAGE_TYPES_H_
