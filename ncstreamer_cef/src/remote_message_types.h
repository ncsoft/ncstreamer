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
    kSettingsWebcamSearchRequest = 501,
    kSettingsWebcamSearchResponse,
    kSettingsWebcamOnRequest = 511,
    kSettingsWebcamOnResponse,
    kSettingsWebcamOffRequest = 521,
    kSettingsWebcamOffResponse,
    kSettingsWebcamSizeRequest = 531,
    kSettingsWebcamSizeResponse,
    kSettingsWebcamPositionRequest = 541,
    kSettingsWebcamPositionResponse,
    kSettingsChromaKeyOnRequest = 601,
    kSettingsChromaKeyOnResponse,
    kSettingsChromaKeyOffRequest = 611,
    kSettingsChromaKeyOffResponse,
    kSettingsChromaKeyColorRequest = 621,
    kSettingsChromaKeyColorResponse,
    kSettingsChromaKeySimilarityRequest = 631,
    kSettingsChromaKeySimilarityResponse,
    kSettingsMicOnRequest = 701,
    kSettingsMicOnResponse,
    kSettingsMicOffRequest = 711,
    kSettingsMicOffResponse,
    kSettingsMicSearchRequest = 721,
    kSettingsMicSearchResponse,
    kNcStreamerUrlUpdateRequest = 731,
    kNcStreamerUrlUpdateResponse,
    kStreamingViewersRequest = 801,
    kStreamingViewersResponse,
    kNcStreamerExitRequest = 901,
    kNcStreamerExitResponse,  // not used.
  };
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_REMOTE_MESSAGE_TYPES_H_
