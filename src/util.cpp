#include "util.h"
#include "domain-types.h"
#include "const-strings.h"
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

    void addMember(const char* key, const string& value)
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

    void pushBack(const string& value)
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

    void addMember(const char* key, const string& value)
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

    void pushBack(const string& value)
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

    string toString() const
    {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc_.Accept(writer);

        return buffer.GetString();
    }

private:
    rapidjson::Document doc_;
    rapidjson::Document::AllocatorType& allocator_;
};

string toJsonString(const Instrument& instrument)
{
    JsonDoc doc;
    doc.addMember(kStrName, instrument.name);
    doc.addMember(kStrInstrumentType, instrument.type);
    return doc.toString();
}

string mimeTypeFromFilePath(const string& filePath)
{
    typedef map<string, string> mapss_t;
    static mapss_t extToMime;

    if (extToMime.empty()) {
        extToMime["png"] = "image/png";
        extToMime["jpg"] = "image/jpeg";
        extToMime["jpeg"] = "image/jpeg";
        extToMime["mp4"] = "video/mp4";
        extToMime["h264"] = "video/h264";
    }

    size_t extPos = filePath.find_last_of('.');

    if (extPos == string::npos)
        return "";

    mapss_t::const_iterator cit = extToMime.find(filePath.substr(extPos+1));

    if (cit == extToMime.end())
        return "";

    return cit->second;
}

static const char* kFullTimeFormat = "%Y-%m-%dT%H:%M:%S";
static const size_t kFullTimeStrlen = 20; // Length of "2016-02-08T16:15:20\0"

string toIsoTimeString(const timestamp_t& timestamp)
{
    time_t time = system_clock::to_time_t(timestamp);
    tm* utcTime = gmtime(&time);
    char timeBuffer[kFullTimeStrlen];
    strftime(timeBuffer, kFullTimeStrlen, kFullTimeFormat, utcTime);
    string rv(timeBuffer);
    int milliseconds = (timestamp.time_since_epoch().count() / 1000LL) % 1000LL;
    rv.append(".");
    // TODO fix milliseconds must output as %03d
    rv.append(toString(milliseconds));

    LINFO << "toIsoTimeString: " << rv;

    return rv;
}

string toString(int value)
{
    const size_t bufSize = 16;
    char buf[bufSize];
    snprintf(buf, bufSize, "%d", value);
    return string(buf);
}

string toJsonString(const EventData& data)
{
    JsonDoc doc(true);
    rapidjson::Document::AllocatorType& allocator = doc.rawRef().GetAllocator();

    for (size_t i = 0; i < data.size(); ++i)
    {
        JsonValue obj(allocator);
        obj.addMember(kStrTimestamp, toIsoTimeString(data[i].timestamp));
        doc.pushBack(obj);
    }

    return doc.toString();
}

string toJsonString(const ObjectStream& os)
{
    JsonDoc doc;

    doc.addMember(kStrCollected, toIsoTimeString(os.collected));
    doc.addMember(kStrLocationX, os.locationX);
    doc.addMember(kStrLocationY, os.locationY);
    doc.addMember(kStrWidth, os.width);
    doc.addMember(kStrHeight, os.height);
    doc.addMember(kStrOrigImageWidth, os.origImageWidth);
    doc.addMember(kStrOrigImageHeight, os.origImageHeight);
    doc.addMember(kStrObjectId, os.objectId);
    doc.addMember(kStrStreamType, os.streamType);

    return doc.toString();
}

}
}
