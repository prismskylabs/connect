#ifndef CONNECT_SDK_UTIL_H
#define CONNECT_SDK_UTIL_H

#include "common-types.h"
#include "domain-types.h"

namespace prism
{
namespace connect
{
    string toJsonString(const Instrument&);
    string toJsonString(const EventData&);
    string toJsonString(const ObjectStream&);

    string toString(int value);

    string mimeTypeFromFilePath(const string& fileName);
    string toIsoTimeString(const timestamp_t& timestamp);
}
}

#endif // CONNECT_SDK_UTIL_H
