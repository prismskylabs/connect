/*
 * Copyright (C) 2016-2017 Prism Skylabs
 */
#ifndef CONNECT_SDK_DOMAIN_TYPES_H
#define CONNECT_SDK_DOMAIN_TYPES_H

#include <string>
#include <vector>
#include <iostream>

#include "common-types.h"

namespace prism
{
namespace connect
{

typedef int32_t id_t;

class Status
{
public:
    Status(int code, bool isError, int facility)
    {
        assert(code - (code & CODE_MASK) == 0);
        assert(facility - (facility & FACILITY_MASK) == 0);

        status_ = ((uint32_t)(isError ? 1 : 0) << 31) | ((uint32_t)facility << 16) | (uint32_t)code;
    }

    // codes
    enum
    {
        SUCCESS = 0,
        FAILURE = 1 // for any (unknown) reason

        // extend with more specific codes as necessary
    };

    enum
    {
        FACILITY_NONE = 0, // for general codes
        FACILITY_NETWORK = 1,
        FACILITY_WEBAPI = 2

        // extend with more facilities as necessary
    };

    int getCode() const
    {
        return status_ & CODE_MASK;
    }

    int getFacility() const
    {
        return (status_ >> 16) & FACILITY_MASK;
    }

    bool isError() const
    {
        return (status_ >> 31) == 1;
    }

    bool isSuccess() const
    {
        return !isError();
    }

    friend std::ostream& operator<<(std::ostream&, const Status&);
private:
    enum
    {
        CODE_MASK = 0xffff,
        FACILITY_MASK = 0x7ff
    };

    // borrowed from Windows HRESULT
    // isError : 1 aka severity
    // reserved : 4
    // facility : 11 ~2k values
    // code : 16 ~65k values

    uint32_t status_;
};

std::ostream& operator<<(std::ostream& os, const Status& status);

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
    Count(const timestamp_t& timestamp, int32_t value)
        : timestamp(timestamp)
        , value(value)
    {
    }

    timestamp_t timestamp;
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
        , data(0)
        , dataSize(0)
    {
    }

    // Caller owns the data. Data shall be available until function accepting
    // parameter of type Payload returns.
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

class PayloadAu;
typedef boost::shared_ptr<PayloadAu> PayloadAuPtr;

}
}

#endif // CONNECT_SDK_DOMAIN_TYPES_H
