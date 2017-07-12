/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_CEF_SRC_STREAMING_SERVICE_STREAMING_SERVICE_TYPES_H_
#define NCSTREAMER_CEF_SRC_STREAMING_SERVICE_STREAMING_SERVICE_TYPES_H_


#include <string>
#include <unordered_map>


namespace ncstreamer {
using SourceTagMap = std::unordered_map<
        std::string /*source_title*/, std::string /*tag_id*/>;

using StreamingServiceTagMap = std::unordered_map<
    std::string /*service_provider_id*/, SourceTagMap>;
}  // namespace ncstreamer


#endif  // NCSTREAMER_CEF_SRC_STREAMING_SERVICE_STREAMING_SERVICE_TYPES_H_
