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

struct SdkVersion
{
    // ugly workaround for ugly bug in gcc, defining macros major() and minor()
#undef major
#undef minor

    SdkVersion(uint8_t major, uint8_t minor, uint8_t revision)
        : major(major)
        , minor(minor)
        , revision(revision)
        , unused(0)
    {
    }

    uint32_t toUint() const
    {
        return ((uint32_t)major << 24) + ((uint32_t)minor << 16) + ((uint32_t)revision << 8);
    }

    std::string toString() const;

    uint8_t major;
    uint8_t minor;
    uint8_t revision;
    uint8_t unused;
};

SdkVersion getSdkVersion();

// client interface reflects Prism Connect Device API v1.0
// see https://github.com/prismskylabs/connect/wiki/Prism-Connect-Device-API-v1.0
class Client
{
public:

    Client(const std::string& apiRoot, const std::string& token);
    ~Client();

    // init() method is synchronous regardless of other methods
    Status init();

    // default is 300000 (300 sec), pass 0 to reset to default
    void setConnectionTimeoutMs(long timeoutMs);

    // Call to make API abort connection, if average transfer speed,
    // bytes per second, is below lowSpeedLimit for lowSpeedTime seconds.
    // To disable, call setLowSpeed(0, 0)
    void setLowSpeed(long lowSpeedTime, long lowSpeedLimit = 1);

    // By default peer is verified when using SSL, use this method to enable/disable
    // verification
    void setSslVerifyPeer(bool sslVerifyPeer);

    Status queryApiState(std::string& accountsUrl, std::string& apiVersion);

    Status queryAccountsList(Accounts& accounts);

    Status queryAccount(id_t accountId, Account& account);

    Status queryInstrumentsList(id_t accountId, Instruments& instruments);

    // not sure we can get instrument by ID, as there is no such member for now
//    Status queryInstrument(id_t accountId, id_t instrumentId, Instrument& instrument);

    Status registerInstrument(id_t accountId, const Instrument& instrument);

    // image uploads
    Status uploadBackground(id_t accountId, id_t instrumentId,
                              const timestamp_t& timestamp, const Payload& payload);

    Status uploadTapestry(id_t accountId, id_t instrumentId,
                            const timestamp_t& eventTimestamp, const Payload& payload,
                            const std::string& type = "SUMMARY_PORTRAIT");

    Status uploadLiveTile(id_t accountId, id_t instrumentId,
                            const timestamp_t& eventTimestamp, const Payload& payload);

    Status uploadObjectStream(id_t accountId, id_t instrumentId,
                                const ObjectStream& stream, const Payload& payload);

    // video uploads
    Status uploadVideo(id_t accountId, id_t instrumentId,
                         const timestamp_t& startTimestamp, const timestamp_t& stopTimestamp,
                         const Payload& payload);

    Status uploadLiveLoop(id_t accountId, id_t instrumentId,
                            const timestamp_t& startTimestamp, const timestamp_t& stopTimestamp,
                            const Payload& payload);

    Status uploadFlipbook(id_t accountId, id_t instrumentId,
                            const Flipbook& flipbook, const Payload& payload);

    // time-series uploads

    Status uploadCount(id_t accountId, id_t instrumentId,
                       const Counts& data, bool update = true);


    Status uploadEvent(id_t accountId, id_t instrumentId,
                         const timestamp_t& timestamp, const Events& data);

    Status uploadTrack(id_t accountId, id_t instrumentId,
                         const timestamp_t& timestamp, const Tracks& data);

    Status uploadTag(id_t accountId, id_t instrumentId,
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
    unique_ptr<Impl>::t pImpl_;

    Impl& impl()
    {
        return *pImpl_;
    }
};

// On success fills cameraInfo.
Status findCameraByName(Client& client, id_t accountId, const std::string& name,
                        Instrument& cameraInfo);

// On success fills cameraInfo with data about just created camera.
Status registerNewCamera(Client& client, id_t accountId, const std::string& name,
                         Instrument& cameraInfo);



} // namespace connect
} // namespace prism

#endif
