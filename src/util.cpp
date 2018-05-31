/*
 * Copyright (C) 2016-2017 Prism Skylabs
 */
#include "private/util.h"
#include "domain-types.h"
#include "private/const-strings.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "ctime"
#include "easylogging++.h"

namespace prism
{
namespace connect
{

class JsonValue
{
public:
    JsonValue(rapidjson::Document::AllocatorType& allocator,
              bool isArray = false)
        : allocator_(allocator)
    {
        if (isArray)
            value_.SetArray();
        else
            value_.SetObject();
    }

    operator rapidjson::Value& ()
    {
        return value_;
    }

    void addMember(const char* key, rapidjson::Value& objValue)
    {
        value_.AddMember(rapidjson::StringRef(key), objValue, allocator_);
    }

    void addMember(const char* key, const char* strValue)
    {
        rapidjson::Value value;
        value.SetString(strValue, allocator_);
        value_.AddMember(rapidjson::StringRef(key), value, allocator_);
    }

    void addMember(const char* key, const std::string& value)
    {
        addMember(key, value.c_str());
    }

    void addMember(const char* key, int intValue)
    {
        rapidjson::Value value;
        value.SetInt(intValue);
        value_.AddMember(rapidjson::StringRef(key), value, allocator_);
    }

    void pushBack(const char* strValue)
    {
        rapidjson::Value value;
        value.SetString(strValue, allocator_);
        value_.PushBack(value, allocator_);
    }

    void pushBack(const std::string& value)
    {
        pushBack(value.c_str());
    }

    void pushBack(int intValue)
    {
        rapidjson::Value value;
        value.SetInt(intValue);
        value_.PushBack(value, allocator_);
    }

    void pushBack(rapidjson::Value& objValue)
    {
        value_.PushBack(objValue, allocator_);
    }

private:
    rapidjson::Value value_;
    rapidjson::Document::AllocatorType& allocator_;
};

class JsonDoc
{
public:
    JsonDoc(bool isArray = false)
        : doc_()
        , allocator_(doc_.GetAllocator())
    {
        if (isArray)
            doc_.SetArray();
        else
            doc_.SetObject();
    }

    rapidjson::Document& rawRef()
    {
        return doc_;
    }

    void addMember(const char* key, rapidjson::Value& objValue)
    {
        doc_.AddMember(rapidjson::StringRef(key), objValue, allocator_);
    }

    void addMember(const char* key, const char* strValue)
    {
        rapidjson::Value value;
        value.SetString(strValue, allocator_);
        doc_.AddMember(rapidjson::StringRef(key), value, allocator_);
    }

    void addMember(const char* key, const std::string& value)
    {
        addMember(key, value.c_str());
    }

    void addMember(const char* key, int intValue)
    {
        rapidjson::Value value;
        value.SetInt(intValue);
        doc_.AddMember(rapidjson::StringRef(key), value, allocator_);
    }

    void pushBack(const char* strValue)
    {
        rapidjson::Value value;
        value.SetString(strValue, allocator_);
        doc_.PushBack(value, allocator_);
    }

    void pushBack(const std::string& value)
    {
        pushBack(value.c_str());
    }

    void pushBack(int intValue)
    {
        rapidjson::Value value;
        value.SetInt(intValue);
        doc_.PushBack(value, allocator_);
    }

    void pushBack(rapidjson::Value& objValue)
    {
        doc_.PushBack(objValue, allocator_);
    }

    std::string toString() const
    {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc_.Accept(writer);

        return buffer.GetString();
    }

    void reserve(size_t newCapacity)
    {
        doc_.Reserve(newCapacity, allocator_);
    }

private:
    rapidjson::Document doc_;
    rapidjson::Document::AllocatorType& allocator_;
};

std::string toJsonString(const Feed& feed)
{
    JsonDoc doc;
    doc.addMember(kStrName, feed.name);
    return doc.toString();
}

std::string mimeTypeFromFilePath(const std::string& filePath)
{
    typedef std::map<std::string, std::string> mapss_t;
    static mapss_t extToMime;

    if (extToMime.empty()) {
        extToMime["png"] = "image/png";
        extToMime["jpg"] = "image/jpeg";
        extToMime["jpeg"] = "image/jpeg";
        extToMime["mp4"] = "video/mp4";
        extToMime["h264"] = "video/h264";
    }

    size_t extPos = filePath.find_last_of('.');

    if (extPos == std::string::npos)
        return "";

    mapss_t::const_iterator cit = extToMime.find(filePath.substr(extPos+1));

    if (cit == extToMime.end())
        return "";

    return cit->second;
}

//static const char* kFullTimeFormat = "%Y-%m-%dT%H:%M:%S";
//static const char* kFullTimeFormat = "%Y%m%dT%H%M%S.%fZ";
//static const size_t kFullTimeStrlen = 20; // Length of "2016-02-08T16:15:20\0"

std::string toIsoTimeString(const timestamp_t& timestamp)
{
    using boost::chrono::system_clock;
    static system_clock::time_point epochStart = system_clock::from_time_t(0);
    system_clock::time_point now = epochStart + boost::chrono::seconds(timestamp/1000);
    time_t time = system_clock::to_time_t(now);
    tm* utcTime = gmtime(&time);
    const size_t bufSize = 32;
    char buffer[bufSize];
    int numMs = timestamp % 1000;
    snprintf(buffer, bufSize,
             //"%04d-%02d-%02dT%02d:%02d:%02d.%03d",
             "%04d%02d%02dT%02d%02d%02d.%03dZ",
             utcTime->tm_year + 1900, utcTime->tm_mon + 1, utcTime->tm_mday,
             utcTime->tm_hour, utcTime->tm_min, utcTime->tm_sec, numMs);

    return buffer;
}

std::string toString(int value)
{
    const size_t bufSize = 16;
    char buf[bufSize];
    snprintf(buf, bufSize, "%d", value);
    return std::string(buf);
}

static void toJsonStringBase(const Artifact& artifact, const std::string& contentType, JsonDoc& doc)
{
    doc.addMember(kStrExtId,       artifact.extId);
    doc.addMember(kStrBegin,       artifact.begin);
    doc.addMember(kStrEnd,         artifact.end);
    doc.addMember(kStrContentType, contentType);
}

std::string toJsonString(const Background& background, const std::string& contentType)
{
    JsonDoc doc;
    toJsonStringBase(background, contentType, doc);

    // specific fields
    doc.addMember(kStrFrameWidth, background.frameWidth);
    doc.addMember(kStrFrameHeight, background.frameHeight);

    return doc.toString();
}

std::string toJsonString(const Flipbook& flipbook, const std::string& contentType)
{
    JsonDoc doc;
    toJsonStringBase(flipbook, contentType, doc);

    // specific fields
    doc.addMember(kStrFrameWidth, flipbook.frameWidth);
    doc.addMember(kStrFrameHeight, flipbook.frameHeight);
    doc.addMember(kStrNumberOfFrames, flipbook.numberOfFrames);

    return doc.toString();
}

std::string toJsonString(const ObjectSnapshot& os, const std::string& contentType)
{
    JsonDoc doc;
    toJsonStringBase(os, contentType, doc);

    doc.addMember(kStrLocationX, os.locationX);
    doc.addMember(kStrLocationY, os.locationY);
    doc.addMember(kStrFrameWidth, os.frameWidth);
    doc.addMember(kStrFrameHeight, os.frameHeight);
    doc.addMember(kStrImageWidth, os.imageWidth);
    doc.addMember(kStrImageHeight, os.imageHeight);

    rapidjson::Document::AllocatorType& allocator = doc.rawRef().GetAllocator();
    JsonValue jsonObjectIds(allocator, true);

    for (size_t j = 0; j < os.objectIds.size(); ++j)
    {
        jsonObjectIds.pushBack(os.objectIds[j]);
    }

    doc.addMember(kStrObjectIds, jsonObjectIds);

    return doc.toString();
}

std::string toJsonString(const Track& track, const std::string& contentType)
{
    JsonDoc doc;
    rapidjson::Document::AllocatorType& allocator = doc.rawRef().GetAllocator();

    toJsonStringBase(track, contentType, doc);

    doc.addMember(kStrObjectId, std::to_string(track.objectId));

    JsonValue jsonPoints(allocator, true);

    for (size_t j = 0; j < track.points.size(); ++j)
    {
        JsonValue jsonPoint(allocator, true);
        const TrackPoint& tp = track.points[j];

        jsonPoint.pushBack(tp.x);
        jsonPoint.pushBack(tp.y);
        jsonPoint.pushBack(tp.relativeTimeMs);

        jsonPoints.pushBack(jsonPoint);
    }

    doc.addMember(kStrPoints, jsonPoints);

    return doc.toString();
}

std::string toJsonString(const TimeSeries& series, const std::string& contentType)
{
    JsonDoc doc;
    rapidjson::Document::AllocatorType& allocator = doc.rawRef().GetAllocator();

    toJsonStringBase(series, contentType, doc);

    doc.addMember(kStrLabel, series.label);

    JsonValue jsonShape(allocator, true);
    for (size_t i = 0; i < series.shape.size(); ++i)
        jsonShape.pushBack(series.shape[i]);
    doc.addMember(kStrShape, jsonShape);

    return doc.toString();
}

static std::string toStringBase(const Artifact& a)
{
    std::stringstream ss;

    ss << "ext_id = " << a.extId
       << ", begin = " << toIsoTimeString(a.begin)
       << ", end = " << toIsoTimeString(a.end);

    return ss.str();
}

std::string toString(const Payload& payload)
{
    std::stringstream ss;

    if (payload.data)
    {
        ss << "payload{dataSize = " << payload.dataSize
           << ", mimeType = " << payload.mimeType << "}";
    }
    else
        ss << "payload{fileName = " << payload.fileName << "}";

    return ss.str();
}

std::string toString(const Background& bg)
{
    std::stringstream ss;

    ss << "background{"
       << toStringBase(bg)
       << "h = " << bg.frameHeight << ", w = " << bg.frameWidth << "}";

    return ss.str();
}

std::string toString(const Flipbook& fb)
{
    std::stringstream ss;

    ss << "flipbook{"
       << toStringBase(fb)
       << "h = " << fb.frameHeight << ", w = " << fb.frameWidth
       << ", frames = " << fb.numberOfFrames << "}";

    return ss.str();
}

std::string toString(const ObjectSnapshot& os)
{
    std::stringstream ss;

    ss << "objectSnapshot{"
       << toStringBase(os)
       << ", ids = [";
    for (size_t i = 0; i < os.objectIds.size(); ++i)
    {
        ss << os.objectIds[i];
        if (i + 1 < os.objectIds.size())
            ss << ", ";
    }
    ss << "]";
    ss << ", y = " << os.locationY << ", x = " << os.locationX
       << ", h = " << os.frameHeight << ", w = " << os.frameWidth
       << ", image height = " << os.imageHeight
       << ", image width = " << os.imageWidth;
    ss << "}";

    return ss.str();
}

std::string toString(const TrackPoint& tp)
{
    std::stringstream ss;
    ss << "[" << tp.x << ", " << tp.y << ", " << tp.relativeTimeMs << "]";
    return ss.str();
}

std::string toString(const Track& track)
{
    std::stringstream ss;

    ss << "track{"
       << toStringBase(track)
       << ", objectId: " << track.objectId
       << ", h = " << track.frameHeight << ", w = " << track.frameWidth
       << ", points{size: " << track.points.size() << ", [";

    for (size_t j = 0; j < track.points.size(); ++j)
        ss << (j ? ", " : "") << toString(track.points[j]);

    ss << "]}";

    return ss.str();
}

std::string toString(const TimeSeriesData& seriesData)
{
    std::stringstream ss;

    ss << "[";
    for (size_t j = 0; j < seriesData.values.size(); ++j)
        ss << (j ? ", " : "") << toString(seriesData.values[j]);
    ss << "], " << seriesData.timeDeltaMs << "]";

    return ss.str();
}

std::string toString(const TimeSeries& series)
{
    std::stringstream ss;

    ss << "series{"
       << toStringBase(series);

    ss << ", label: " << series.label;
    ss << ", shape{[";
    for (size_t j = 0; j < series.shape.size(); ++j)
        ss << (j ? ", " : "") << toString(series.shape[j]);
    ss << "]}";

    ss << ", data{[";
    for (size_t j = 0; j < series.data.size(); ++j)
        ss << (j ? ", " : "") << toString(series.data[j]);
    ss << "]}}";

    return ss.str();
}

}
}
