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

// client interface reflects Prism Connect Device API v2.0.1
// see https://github.com/prismskylabs/prismai_web/wiki/Prism-Connect-API-v2.0.1
class Client
{
public:

    Client(const std::string& apiRoot = "", const std::string& token = "");
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

    Status queryFeedsList(id_t accountId, Feeds& feeds);

    Status registerFeed(id_t accountId, const Feed& feed);

    // image uploads
    Status uploadBackground(id_t accountId, id_t feedId, const Background& background, const Payload& payload);
    Status uploadFlipbook(id_t accountId, id_t feedId, const Flipbook& flipbook, const Payload& payload);
    Status uploadObjectSnapshot(id_t accountId, id_t feedId, const ObjectSnapshot& snapshot, const Payload& payload);

    // time-series uploads
    Status uploadTrack(id_t accountId, id_t feedId, const Track& track);
    Status uploadTimeSeries(id_t accountId, id_t feedId, const TimeSeries& series);

    enum
    {
        // log input parameters of client methods
        LOG_INPUT       = 0x00000001,

        // in case JSON is generated from one or more parameters, logs that JSON
        LOG_INPUT_JSON  = 0x00000002,

        LOG_RESPONSE    = 0x00000004
    };

    // LOG_ values combined with OR
    // use it to selectively enable or disable (logFlags = 0 disables all) logging
    void setLogFlags(int logFlags);

    // Proxy is of format [<scheme>://]<host>[:<port>] e.g. "http://proxy:80"
    // <scheme> is one of http, https, socks4, socks4a, socks5, socks5a
    void setProxy(const std::string& proxy);

    void swap(Client& other)
    {
        pImpl_.swap(other.pImpl_);
    }

    // By default, caBundlePath is set to empty string. Use
    // to specify path to Certificate Authority (CA) bundle
    void setCaBundlePath(const std::string& caBundlePath);

private:
    class Impl;
    unique_ptr<Impl>::t pImpl_;

    Impl& impl()
    {
        return *pImpl_;
    }
};

inline void swap(Client& one, Client& two)
{
    one.swap(two);
}

// On success fills cameraInfo.
Status findCameraByName(Client& client, id_t accountId, const std::string& name,
                        Feed& cameraInfo);

// On success fills cameraInfo with data about just created camera.
Status registerNewCamera(Client& client, id_t accountId, const std::string& name,
                         Feed& cameraInfo);

// Aggregates client and both IDs. Nothing more.
struct ClientSession
{
    ClientSession()
        : accountId(-1)
        , cameraId(-1)
    {
    }

    Client client;
    id_t accountId;
    id_t cameraId;
};


} // namespace connect
} // namespace prism

#endif
