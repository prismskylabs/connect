#ifndef CONNECT_H
#define CONNECT_H

#include <string>
#include <vector>
#include <map>
#include <boost/move/unique_ptr.hpp>
#include <boost/chrono/system_clocks.hpp>

namespace prism {
namespace connect {

using std::string;
using std::vector;
using boost::movelib::unique_ptr;
using boost::chrono::system_clock;

typedef int32_t accountId_t;
typedef int32_t instrumentId_t;

struct Account {
    accountId_t id;
    string      name;
    string      instrumentsUrl;
    string      url;
};

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
    string                  name;
    string                  type;
    InstrumentConfiguration config;
    Metadata                metadata;
    string                  externalId;
    string                  externalDeviceId;
};

typedef vector<Account> AccountsList;
typedef vector<Instrument> InstrumentsList;

enum {
    GENTS_INCLUDE_MILLISECONDS  = 1,
    GENTS_SECONDS_ARE_ZERO      = 2,
    GENTS_DEFAULT               = GENTS_SECONDS_ARE_ZERO
};

// you are free to use your own implementation for obtaining timestamp
// and converting it to string
string generateTimestamp(bool includeMilliseconds = false);

// client interface reflects Prism Connect Device API v1.0
// see https://github.com/prismskylabs/connect/wiki/Prism-Connect-Device-API-v1.0
class Client {
public:
    typedef int status_t;

    enum {
        STATUS_OK       = 0,
        STATUS_ERROR    = 1
    };

    Client(const string& apiRoot, const string& token);
    ~Client();

    status_t init();

    status_t queryApiState(string& accountsUrl, string& apiVersion);

    status_t queryAccountsList(AccountsList& accounts);

    status_t queryAccount(accountId_t accountId, Account& account);

    status_t queryInstrumentsList(accountId_t accountId, InstrumentsList& instruments);

    status_t queryInstrument(accountId_t accountId, instrumentId_t instrumentId,
                             Instrument& instrument);

    status_t registerInstrument(Instrument& instrument);

    // image uploads
    status_t uploadBackground(accountId_t accountId, instrumentId_t instrumentId,
                              const string& timestamp, const string& imageFile);

    status_t uploadTapestry(accountId_t accountId, instrumentId_t instrumentId,
                            const string& eventTimestamp, const string& imageFile,
                            const string& type = "SUMMARY_PORTRAIT");

    status_t uploadLiveTile(accountId_t accountId, instrumentId_t instrumentId,
                            const string& eventTimestamp, const string& imageFile);

    status_t uploadObjectStream(accountId_t accountId, instrumentId_t instrumentId,
                                const Metadata& metadata, const string& imageFile);

    // video uploads
    status_t uploadVideo(accountId_t accountId, instrumentId_t instrumentId,
                         const string& startTimestamp, const string& stopTimestamp,
                         const string& videoFile);

    status_t uploadLiveloop(accountId_t accountId, instrumentId_t instrumentId,
                            const string& startTimestamp, const string& stopTimestamp,
                            const string& videoFile);

    status_t uploadFlipbook(accountId_t accountId, instrumentId_t instrumentId,
                            const string& startTimestamp, const string& stopTimestamp,
                            const string& videoFile, int32_t width, int32_t height,
                            int32_t numberOfFrames);

    // time-series uploads
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

    status_t uploadCount(accountId_t accountId, instrumentId_t instrumentId,
                         const string& timestamp, const CountData& data,
                         bool update = true);

    struct EventItem {

    };

    typedef vector<EventItem> EventData;

    status_t uploadEvent(accountId_t accountId, instrumentId_t instrumentId,
                         const string& timestamp, const EventData& data);

    struct TrackItem {

    };

    typedef vector<TrackItem> TrackData;

    status_t uploadTrack(accountId_t accountId, instrumentId_t instrumentId,
                         const string& timestamp, const TrackData& data);

    struct TagItem {

    };

    typedef vector<TagItem> TagData;

    status_t uploadTag(accountId_t accountId, instrumentId_t instrumentId,
                       const string& timestamp, const TagData& data);

private:
    class Impl;
    unique_ptr<Impl> pImpl_;
};

} // namespace connect
} // namespace prism

#endif
