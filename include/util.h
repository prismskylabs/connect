/*
 * Copyright (C) 2016-2017 Prism Skylabs
 */
#ifndef CONNECT_SDK_UTIL_H
#define CONNECT_SDK_UTIL_H

#include "common-types.h"
#include "domain-types.h"

namespace prism
{
namespace connect
{
    std::string toJsonString(const Instrument&);
    std::string toJsonString(const EventData&);
    std::string toJsonString(const ObjectStream&);

    std::string toString(int value);

    std::string mimeTypeFromFilePath(const std::string& fileName);
    std::string toIsoTimeString(const timestamp_t& timestamp);

    timestamp_t toTimestamp(const boost::chrono::system_clock::time_point& timePoint);
}
}

#endif // CONNECT_SDK_UTIL_H
