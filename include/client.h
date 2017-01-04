/*
 * Copyright (C) 2017 Prism Skylabs
 *
 * Licensed under 3-clause BSD license
 */
#ifndef CONNECT_SDK_CLIENT_H
#define CONNECT_SDK_CLIENT_H

#include "domain-types.h"

namespace prism {
namespace connect {

// client interface reflects Prism Connect Device API v1.0
// see https://github.com/prismskylabs/connect/wiki/Prism-Connect-Device-API-v1.0
class Client {
public:

    Client(const string& apiRoot, const string& token);
    ~Client();

    // init() method is synchronous regardless of other methods
    status_t init();

    status_t queryApiState(string& accountsUrl, string& apiVersion);

    status_t queryAccountsList(AccountsList& accounts);

    status_t queryAccount(id_t accountId, Account& account);

    status_t queryInstrumentsList(id_t accountId, InstrumentsList& instruments);

    // not sure we can get instrument by ID, as there is no such member for now
//    status_t queryInstrument(id_t accountId, id_t instrumentId, Instrument& instrument);

    status_t registerInstrument(id_t accountId, const Instrument& instrument);

    // image uploads
    status_t uploadBackground(id_t accountId, id_t instrumentId,
                              const timestamp_t& timestamp, const string& imageFile);

    status_t uploadTapestry(id_t accountId, id_t instrumentId,
                            const timestamp_t& eventTimestamp, const string& imageFile,
                            const string& type = "SUMMARY_PORTRAIT");

    status_t uploadLiveTile(id_t accountId, id_t instrumentId,
                            const timestamp_t& eventTimestamp, const string& imageFile);

    status_t uploadObjectStream(id_t accountId, id_t instrumentId,
                                const ObjectStream& stream, const string& imageFile);

    // video uploads
    status_t uploadVideo(id_t accountId, id_t instrumentId,
                         const timestamp_t& startTimestamp, const timestamp_t& stopTimestamp,
                         const string& videoFile);

    status_t uploadLiveLoop(id_t accountId, id_t instrumentId,
                            const timestamp_t& startTimestamp, const timestamp_t& stopTimestamp,
                            const string& videoFile);

    status_t uploadFlipbook(id_t accountId, id_t instrumentId,
                            const Flipbook& flipbook);

    // time-series uploads

    status_t uploadCount(id_t accountId, id_t instrumentId,
                         const timestamp_t& timestamp, const CountData& data,
                         bool update = true);


    status_t uploadEvent(id_t accountId, id_t instrumentId,
                         const timestamp_t& timestamp, const EventData& data);

    status_t uploadTrack(id_t accountId, id_t instrumentId,
                         const timestamp_t& timestamp, const TrackData& data);

    status_t uploadTag(id_t accountId, id_t instrumentId,
                       const timestamp_t& timestamp, const TagData& data);

private:
    class Impl;
    Impl* pImpl_;
};

} // namespace connect
} // namespace prism

#endif
