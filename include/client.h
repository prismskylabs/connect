/*
 * Copyright (C) 2016-2017 Prism Skylabs
 */
#ifndef CONNECT_SDK_CLIENT_H
#define CONNECT_SDK_CLIENT_H

#include "domain-types.h"

namespace prism
{
namespace connect
{

// client interface reflects Prism Connect Device API v1.0
// see https://github.com/prismskylabs/connect/wiki/Prism-Connect-Device-API-v1.0
class Client
{
public:

    Client(const std::string& apiRoot, const std::string& token);
    ~Client();

    // init() method is synchronous regardless of other methods
    status_t init();

    // default is 300000 (300 sec), pass 0 to reset to default
    void setConnectionTimeoutMs(long timeoutMs);

    // Call to make API abort connection, if transfer speed is below lowSpeedLimit
    // for lowSpeedTime seconds.
    // This is only for established connections, see also setConenctionTimeout*().
    // To disable, call setLowSpeed(0, 0)
    void setLowSpeed(long lowSpeedTime, long lowSpeedLimit = 1);

    status_t queryApiState(std::string& accountsUrl, std::string& apiVersion);

    status_t queryAccountsList(Accounts& accounts);

    status_t queryAccount(id_t accountId, Account& account);

    status_t queryInstrumentsList(id_t accountId, Instruments& instruments);

    // not sure we can get instrument by ID, as there is no such member for now
//    status_t queryInstrument(id_t accountId, id_t instrumentId, Instrument& instrument);

    status_t registerInstrument(id_t accountId, const Instrument& instrument);

    // image uploads
    status_t uploadBackground(id_t accountId, id_t instrumentId,
                              const timestamp_t& timestamp, const Payload& payload);

    status_t uploadTapestry(id_t accountId, id_t instrumentId,
                            const timestamp_t& eventTimestamp, const Payload& payload,
                            const std::string& type = "SUMMARY_PORTRAIT");

    status_t uploadLiveTile(id_t accountId, id_t instrumentId,
                            const timestamp_t& eventTimestamp, const Payload& payload);

    status_t uploadObjectStream(id_t accountId, id_t instrumentId,
                                const ObjectStream& stream, const Payload& payload);

    // video uploads
    status_t uploadVideo(id_t accountId, id_t instrumentId,
                         const timestamp_t& startTimestamp, const timestamp_t& stopTimestamp,
                         const Payload& payload);

    status_t uploadLiveLoop(id_t accountId, id_t instrumentId,
                            const timestamp_t& startTimestamp, const timestamp_t& stopTimestamp,
                            const Payload& payload);

    status_t uploadFlipbook(id_t accountId, id_t instrumentId,
                            const Flipbook& flipbook, const Payload& payload);

    // time-series uploads

    status_t uploadCount(id_t accountId, id_t instrumentId,
                         const timestamp_t& timestamp, const Counts& data,
                         bool update = true);


    status_t uploadEvent(id_t accountId, id_t instrumentId,
                         const timestamp_t& timestamp, const Events& data);

    status_t uploadTrack(id_t accountId, id_t instrumentId,
                         const timestamp_t& timestamp, const Tracks& data);

    status_t uploadTag(id_t accountId, id_t instrumentId,
                       const timestamp_t& timestamp, const Tags& data);

    enum
    {
        // log input parameters of client methods
        LOG_INPUT       = 0x00000001,

        // in case JSON is generated from one or more parameters, logs that JSON
        LOG_INPUT_JSON  = 0x00000002
    };

    // LOG_ values combined with OR
    // use it to selectively enable or disable (logFlags = 0 disables all) logging
    void setLogFlags(int logFlags);

private:
    class Impl;
    Impl* pImpl_;
};

} // namespace connect
} // namespace prism

#endif
