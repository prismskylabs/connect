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
        FAILURE = 1, // for any (unknown) reason
        NOT_FOUND = 2,
        ALREADY_EXISTS = 3

        // extend with more specific codes as necessary
    };

    enum
    {
        FACILITY_NONE = 0, // for general codes
        FACILITY_NETWORK = 1,
        FACILITY_WEBAPI = 2,
        FACILITY_HTTP = 3

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

    std::string toString() const;

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
    void clear()
    {
        id = -1;
        name.clear();
    }

    id_t        id;
    std::string name;
};

typedef std::vector<Account> Accounts;

struct Feed
{
    struct Configuration
    {
        int dummy;

        void clear()
        {
            dummy = 0;
        }
    };

    struct Metadata
    {
        void clear()
        {
            dummy = 0;
        }

        int dummy;
    };

    void clear()
    {
        id = -1;
        name.clear();
    }

    // all but name and type are ignored for now
    id_t        id;
    std::string name;
};

typedef std::vector<Feed> Feeds;

struct Artifact
{
    std::string extId;
    timestamp_t begin;
    timestamp_t end;

    Artifact()
    {
    }

    Artifact(const std::string& extId, timestamp_t begin, timestamp_t end)
        : extId(extId)
        , begin(begin)
        , end(end)
    {
    }
};

struct Background: public Artifact
{
    int32_t frameWidth;
    int32_t frameHeight;

    Background()
    {
    }

    Background(const std::string& extId, timestamp_t begin, timestamp_t end,
               int32_t frameWidth, int32_t frameHeight)
        : Artifact(extId, begin, end)
        , frameWidth(frameWidth)
        , frameHeight(frameHeight)
    {
    }
};

struct Flipbook: public Artifact
{
    int32_t frameWidth;
    int32_t frameHeight;
    int32_t numberOfFrames;

    Flipbook()
    {
    }

    Flipbook(const std::string& extId, timestamp_t begin, timestamp_t end,
             int32_t frameWidth, int32_t frameHeight, int32_t numberOfFrames)
        : Artifact(extId, begin, end)
        , frameWidth(frameWidth)
        , frameHeight(frameHeight)
        , numberOfFrames(numberOfFrames)
    {
    }
};

struct ObjectSnapshot: public Artifact
{
    int32_t     locationX;
    int32_t     locationY;
    int32_t     frameWidth;
    int32_t     frameHeight;
    int32_t     imageWidth;
    int32_t     imageHeight;
    std::vector<int64_t> objectIds;

    ObjectSnapshot()
    {
    }

    ObjectSnapshot(const std::string& extId, timestamp_t begin, timestamp_t end,
                   int32_t locationX, int32_t locationY, int32_t frameWidth, int32_t frameHeight,
                   int32_t imageWidth, int32_t imageHeight, const std::vector<int64_t>& objectIds)
        : Artifact(extId, begin, end)
        , locationX(locationX)
        , locationY(locationY)
        , frameWidth(frameWidth)
        , frameHeight(frameHeight)
        , imageWidth(imageWidth)
        , imageHeight(imageHeight)
        , objectIds(objectIds)
    {
    }
};

struct TrackPoint
{
    TrackPoint(int x, int y, int relativeTimeMs)
        : x(x)
        , y(y)
        , relativeTimeMs(relativeTimeMs)
    {
    }

    int x;
    int y;
    int relativeTimeMs;
};

typedef std::vector<TrackPoint> TrackPoints;

struct Track: public Artifact
{
    Track()
    {
    }

    Track(int64_t objectId, int32_t frameWidth, int32_t frameHeight, const TrackPoints& points)
        : objectId(objectId)
        , frameWidth(frameWidth)
        , frameHeight(frameHeight)
        , points(points)
    {
    }

    int64_t objectId;
    int32_t frameWidth;
    int32_t frameHeight;
    TrackPoints points;
};

struct TimeSeriesData
{
    TimeSeriesData(const std::vector<float>& values, int timeDeltaMs)
        : values(values)
        , timeDeltaMs(timeDeltaMs)
    {
    }

    std::vector<float> values;
    int timeDeltaMs;
};

struct TimeSeries: public Artifact
{
    TimeSeries()
    {
    }

    TimeSeries(const std::string& extId, timestamp_t begin, timestamp_t end,
               const std::string& label, const std::vector<int32_t>& shape,
               const std::vector<TimeSeriesData>& data)
        : Artifact(extId, begin, end)
        , label(label)
        , shape(shape)
        , data(data)
    {
    }

    int32_t value;
    std::string label;
    std::vector<int32_t> shape;
    std::vector<TimeSeriesData> data;
};

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

}
}

#endif // CONNECT_SDK_DOMAIN_TYPES_H
