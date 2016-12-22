#ifndef CONNECT_SDK_DOMAIN_TYPES_H
#define CONNECT_SDK_DOMAIN_TYPES_H

#include "common-types.h"

namespace prism {
namespace connect {

typedef int32_t id_t;

typedef int status_t;

enum {
    STATUS_OK       = 0,
    STATUS_ERROR    = 1
};

struct Account {
    id_t    id;
    string  name;
    string  instrumentsUrl;
    string  url;
};

typedef vector<Account> AccountsList;

struct InstrumentConfiguration {
    string  macAddress;
    string  timeZone;
};

class Metadata {
public:
    Metadata();
    ~Metadata();

    enum Type {
        INTEGER = 0,
        STRING  = 1,
        DOUBLE  = 2,
    };

    // order of fields in JSON may be differ from order in which
    // values were set (added)
    void setValue(const string& name, int32_t value);
    void setValue(const string& name, const string& value);
    void setValue(const string& name, double value);

    // returns false, if there is no value of given type with given name
    bool getValue(const string& name, int32_t& value) const;
    bool getValue(const string& name, string& value) const;
    bool getValue(const string& name, double& value) const;

    typedef std::map<string, int32_t> NameTypeMap;

    // get list of value names and their respective types
    // use it to iterate over all values e.g. during conversion to JSON
    void getNameTypeMap(NameTypeMap& nameTypeMap) const;

private:
    class Impl;
    unique_ptr<Impl> pImpl_;
};

struct Instrument {
    string      name;
    string      type;
    InstrumentConfiguration config;
    Metadata    metadata;
    string      externalId;
    string      externalDeviceId;
};

typedef vector<Instrument> InstrumentsList;

struct CountItem {
    CountItem(const string& timestamp, int32_t value)
        : timestamp(timestamp)
        , value(value)
    {
    }

    string  timestamp;
    int32_t value;
};

typedef vector<CountItem> CountData;

struct EventItem {

};

typedef vector<EventItem> EventData;

struct TrackItem {

};

typedef vector<TrackItem> TrackData;

struct TagItem {

};

typedef vector<TagItem> TagData;

}
}

#endif // CONNECT_SDK_DOMAIN_TYPES_H
