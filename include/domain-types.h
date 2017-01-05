/*
 * Copyright (C) 2016-2017 Prism Skylabs
 */
#ifndef CONNECT_SDK_DOMAIN_TYPES_H
#define CONNECT_SDK_DOMAIN_TYPES_H

#include <string>
#include <vector>

#include "common-types.h"

namespace prism
{
namespace connect
{

typedef int32_t id_t;

typedef int status_t;

enum
{
    STATUS_OK       = 0,
    STATUS_ERROR    = 1
};

struct Account
{
    id_t        id;
    std::string name;
    std::string instrumentsUrl;
    std::string url;
};

typedef std::vector<Account> AccountsList;

struct Instrument
{
    struct Configuration
    {
        int dummy;
    };

    struct Metadata
    {
        int dummy;
    };

    // all but name and type are ignored for now
    id_t        id;
    std::string name;
    std::string type;
    Configuration config;
    Metadata    metadata;
    std::string externalId;
    std::string externalDeviceId;
};

typedef std::vector<Instrument> InstrumentsList;

struct Flipbook
{
    timestamp_t startTimestamp;
    timestamp_t stopTimestamp;
    std::string videoFile;
    int32_t width;
    int32_t height;
    int32_t numberOfFrames;
};

struct Metadata
{

};

struct ObjectStream
{
    timestamp_t collected;
    int32_t     locationX;
    int32_t     locationY;
    int32_t     width;
    int32_t     height;
    int32_t     origImageWidth;
    int32_t     origImageHeight;
    int32_t     objectId;
    std::string streamType;
};

struct CountItem
{
    CountItem(const std::string& timestamp, int32_t value)
        : timestamp(timestamp)
        , value(value)
    {
    }

    std::string  timestamp;
    int32_t value;
};

typedef std::vector<CountItem> CountData;

struct EventItem
{
    timestamp_t timestamp;
};

typedef std::vector<EventItem> EventData;

struct TrackItem
{

};

typedef std::vector<TrackItem> TrackData;

struct TagItem
{

};

typedef std::vector<TagItem> TagData;

}
}

#endif // CONNECT_SDK_DOMAIN_TYPES_H
