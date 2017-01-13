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

typedef std::vector<Account> Accounts;

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

typedef std::vector<Instrument> Instruments;

struct Flipbook
{
    timestamp_t startTimestamp;
    timestamp_t stopTimestamp;
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

struct Count
{
    Count(const std::string& timestamp, int32_t value)
        : timestamp(timestamp)
        , value(value)
    {
    }

    std::string  timestamp;
    int32_t value;
};

typedef std::vector<Count> Counts;

struct Event
{
    timestamp_t timestamp;
};

typedef std::vector<Event> Events;

struct Track
{

};

typedef std::vector<Track> Tracks;

struct Tag
{

};

typedef std::vector<Tag> Tags;

struct Payload
{
    Payload(const std::string& fileName)
        : fileName(fileName)
        , data(nullptr)
        , dataSize(0)
    {
    }

    Payload(const void* data, size_t dataSize, const std::string& mimeType)
        : data(data)
        , dataSize(dataSize)
        , mimeType(mimeType)
    {
    }

    std::string fileName;
    const void* data;
    size_t dataSize;
    std::string mimeType;
};

}
}

#endif // CONNECT_SDK_DOMAIN_TYPES_H
