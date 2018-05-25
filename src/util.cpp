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

static const char* kFullTimeFormat = "%Y-%m-%dT%H:%M:%S";
static const size_t kFullTimeStrlen = 20; // Length of "2016-02-08T16:15:20\0"

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
    snprintf(buffer, bufSize, "%04d-%02d-%02dT%02d:%02d:%02d.%03d",
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

std::string toJsonString(const Counts& data)
{
    JsonDoc doc(true);
    rapidjson::Document::AllocatorType& allocator = doc.rawRef().GetAllocator();

    doc.reserve(data.size());

    for (size_t i = 0; i < data.size(); ++i)
    {
        JsonValue obj(allocator);
        obj.addMember(kStrTimestamp, toIsoTimeString(data[i].timestamp));
        obj.addMember(kStrLabel, data[i].label);
        obj.addMember(kStrValue, data[i].value);
        doc.pushBack(obj);
    }

    return doc.toString();
}

std::string toJsonString(const Tracks& tracks)
{
    JsonDoc doc(true);
    rapidjson::Document::AllocatorType& allocator = doc.rawRef().GetAllocator();

    doc.reserve(tracks.size());

    for (size_t i = 0; i < tracks.size(); ++i)
    {
        JsonValue jsonTrack(allocator);
        const Track track = tracks[i];
        jsonTrack.addMember(kStrObjectId, std::to_string(track.objectId));
        jsonTrack.addMember(kStrTimestamp, toIsoTimeString(track.timestamp));

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

        jsonTrack.addMember(kStrPoints, jsonPoints);
        doc.pushBack(jsonTrack);
    }

    return doc.toString();
}

std::string toJsonString(const ObjectStream& os)
{
    JsonDoc doc;

    doc.addMember(kStrCollected, toIsoTimeString(os.collected));
    doc.addMember(kStrLocationX, os.locationX);
    doc.addMember(kStrLocationY, os.locationY);
    doc.addMember(kStrWidth, os.width);
    doc.addMember(kStrHeight, os.height);
    doc.addMember(kStrOrigImageWidth, os.origImageWidth);
    doc.addMember(kStrOrigImageHeight, os.origImageHeight);
    doc.addMember(kStrObjectId, std::to_string(os.objectId));
    doc.addMember(kStrStreamType, os.streamType);

    return doc.toString();
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

std::string toString(const Flipbook& fb)
{
    std::stringstream ss;

    ss << "flipbook{h = " << fb.height << ", w = " << fb.width
       << ", frames = " << fb.numberOfFrames
       << ", startTS = " << toIsoTimeString(fb.startTimestamp)
       << ", stopTS = " << toIsoTimeString(fb.stopTimestamp);

    ss << "}";

    return ss.str();
}

std::string toString(const Counts& counts)
{
    std::stringstream ss;

    ss << "counts{size = " << counts.size() << " [";

    for (size_t i = 0; i < counts.size(); ++i)
        ss << (i == 0 ? "{" : ", {")
           << toIsoTimeString(counts[i].timestamp)
           << ", "
           << counts[i].value
           << "}";

    ss << "]}";

    return ss.str();
}

std::string toString(const ObjectStream& os)
{
    std::stringstream ss;

    ss << "objectStream{id = " << os.objectId
       << ", collected = " << toIsoTimeString(os.collected)
       << ", y = " << os.locationY << ", x = " << os.locationX
       << ", h = " << os.height << ", w = " << os.width
       << ", image height = " << os.origImageHeight
       << ", image width = " << os.origImageWidth;
    ss << "}";

    return ss.str();
}

std::string toString(const TrackPoint& tp)
{
    std::stringstream ss;
    ss << "[" << tp.x << ", " << tp.y << ", " << tp.relativeTimeMs << "]";
    return ss.str();
}

std::string toString(const Tracks& tracks)
{
    std::stringstream ss;

    ss << "tracks{size: " << tracks.size() << ", [";

    for (size_t i = 0; i < tracks.size(); ++i)
    {
        ss << (i ? ", " : "") << "{";
        const Track& track = tracks[i];

        ss << "objectId: " << track.objectId
           << ", timestamp: " << toIsoTimeString(track.timestamp)
           << ", points{size: " << track.points.size() << ", [";

        for (size_t j = 0; j < track.points.size(); ++j)
            ss << (j ? ", " : "") << toString(track.points[j]);

        ss << "]}";
    }

    return ss.str();
}

}
}
