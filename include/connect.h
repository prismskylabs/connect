#ifndef CONNECT_H
#define CONNECT_H

#include <string>
#include <vector>
#include <map>

namespace prism {
namespace connect {

using std::string;
using std::vector;

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
    // using raw pointer to avoid C++ version dependent pointer type lookup
    Impl* pImpl_;
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

// client interface reflects Prism Connect Device API v1.0
// see https://github.com/prismskylabs/connect/wiki/Prism-Connect-Device-API-v1.0
class Client {
public:
    typedef int status_t;

    enum {
        STATUS_OK = 0
    };

    Client(const string& apiRoot, const string& token);

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
                                );

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
    status_t uploadCount(accountId_t accountId, instrumentId_t instrumentId,
                         const CountData& data, bool update = true);

    status_t uploadEvent(accountId_t accountId, instrumentId_t instrumentId,
                         const string& timestamp, const EventData& data);

    status_t uploadTrack(accountId_t accountId, instrumentId_t instrumentId,
                         const string& timestamp, const TrackData& data);

    status_t uploadTag(accountId_t accountId, instrumentId_t instrumentId,
                       const string& timestamp, const TagData& data);

    // you are free to use your own implementation of getting timestamp and converting
    // it to string
    static string generateTimestamp(bool includeMilliseconds = false);
};

} // namespace connect
} // namespace prism

#endif
