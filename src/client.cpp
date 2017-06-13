/*
 * Copyright (C) 2016-2017 Prism Skylabs
 */
#include "client.h"
#include "private/const-strings.h"
#include "private/curl-session.h"
#include "private/util.h"
#include "easylogging++.h"
#include "rapidjson/document.h"
#include "ConnectSDKConfig.h"
#include <boost/format.hpp>
#include <boost/filesystem.hpp>

namespace prism
{
namespace connect
{


class Client::Impl
{
public:
    Impl(const std::string& apiRoot, const std::string& token)
        : apiRoot_(apiRoot)
        , token_(token)
        , logFlags_(0)
        , connectionTimeoutMs_(0)
        , lowSpeedLimit_(0)
        , lowSpeedTime_(0)
        , sslVerifyPeer_(true)
    {
    }

    Status init();

    void setConnectionTimeoutMs(long timeoutMs)
    {
        connectionTimeoutMs_ = timeoutMs;
    }

    void setLowSpeed(long lowSpeedTime, long lowSpeedLimit)
    {
        lowSpeedTime_ = lowSpeedTime;
        lowSpeedLimit_ = lowSpeedLimit;
    }

    void setSslVerifyPeer(bool sslVerifyPeer)
    {
        sslVerifyPeer_ = sslVerifyPeer;
    }

    Status queryAccountsList(Accounts& accounts);
    Status queryAccount(id_t accountId, Account &account);
    Status queryInstrumentsList(id_t accountId, Instruments& instruments);
    Status registerInstrument(id_t accountId, const Instrument& instrument);

    Status uploadBackground(id_t accountId, id_t instrumentId,
                              const timestamp_t& timestamp, const Payload& payload);

    Status uploadFlipbook(id_t accountId, id_t instrumentId,
                            const Flipbook& flipbook, const Payload& payload);

    Status uploadCount(id_t accountId, id_t instrumentId,
                       const Counts& data, bool update);

    Status uploadEvent(id_t accountId, id_t instrumentId,
                         const timestamp_t& timestamp, const Events& data);

    Status uploadObjectStream(id_t accountId, id_t instrumentId,
                                const ObjectStream& stream, const Payload& payload);

    Status uploadTrack(id_t accountId, id_t instrumentId,
                       const timestamp_t& timestamp, const Tracks& data);

    void setLogFlags(int logFlags)
    {
        logFlags_ = logFlags;
    }

private:
    std::string getInstrumentsUrl(id_t accountId) const;
    std::string getAccountUrl(id_t accountId) const;
    std::string getInstrumentUrl(id_t accountId, id_t instrumentId) const;
    std::string getVideosUrl(id_t accountId, id_t instrumentId) const;
    std::string getImagesUrl(id_t accountId, id_t instrumentId) const;
    std::string getTimeSeriesUrl(id_t accountId, id_t instrumentId) const;

    CurlSessionPtr createSession();

    std::string apiRoot_;
    std::string token_;

    std::string accountsUrl_;
    int logFlags_;
    long connectionTimeoutMs_;
    long lowSpeedLimit_;
    long lowSpeedTime_;
    bool sslVerifyPeer_;
};

Client::Client(const std::string& apiRoot, const std::string& token)
    : pImpl_(new Impl(apiRoot, token))
{
}

Client::~Client()
{
}

Status Client::init()
{
    return impl().init();
}

void Client::setConnectionTimeoutMs(long timeoutMs)
{
    impl().setConnectionTimeoutMs(timeoutMs);
}

void Client::setLowSpeed(long lowSpeedTime, long lowSpeedLimit)
{
    impl().setLowSpeed(lowSpeedTime, lowSpeedLimit);
}

void Client::setSslVerifyPeer(bool sslVerifyPeer)
{
    impl().setSslVerifyPeer(sslVerifyPeer);
}

Status Client::queryApiState(std::string& accountsUrl, std::string& apiVersion)
{
    return makeError();
}

Status Client::queryAccountsList(Accounts& accounts)
{
    return impl().queryAccountsList(accounts);
}

Status Client::queryAccount(id_t accountId, Account &account)
{
    return impl().queryAccount(accountId, account);
}

Status Client::queryInstrumentsList(id_t accountId, Instruments &instruments)
{
    return impl().queryInstrumentsList(accountId, instruments);
}

Status Client::registerInstrument(id_t accountId, const Instrument& instrument)
{
    return impl().registerInstrument(accountId, instrument);
}

Status Client::uploadBackground(id_t accountId, id_t instrumentId,
                                  const timestamp_t& timestamp, const Payload& payload)
{
    return impl().uploadBackground(accountId, instrumentId, timestamp, payload);
}

Status Client::uploadObjectStream(id_t accountId, id_t instrumentId,
                                    const ObjectStream& stream, const Payload& payload)
{
    return impl().uploadObjectStream(accountId, instrumentId, stream, payload);
}

Status Client::uploadFlipbook(id_t accountId, id_t instrumentId,
                                const Flipbook& flipbook, const Payload& payload)
{
    return impl().uploadFlipbook(accountId, instrumentId, flipbook, payload);
}

Status Client::uploadCount(id_t accountId, id_t instrumentId, const Counts& data, bool update)
{
    return impl().uploadCount(accountId, instrumentId, data, update);
}

Status Client::uploadEvent(id_t accountId, id_t instrumentId,
                             const timestamp_t& timestamp, const Events& data)
{
    return impl().uploadEvent(accountId, instrumentId, timestamp, data);
}

Status Client::uploadTrack(id_t accountId, id_t instrumentId, const timestamp_t& timestamp, const Tracks& data)
{
    return impl().uploadTrack(accountId, instrumentId, timestamp, data);
}

void Client::setLogFlags(int logFlags)
{
    impl().setLogFlags(logFlags);
}

bool hasStringMember(const rapidjson::Value& value, const char* name)
{
    return value.HasMember(name)  &&  value[name].IsString();
}

bool hasIntMember(const rapidjson::Value& value, const char* name)
{
    return value.HasMember(name)  &&  value[name].IsInt();
}

Status Client::Impl::init()
{
    CurlSessionPtr sessionPtr = createSession();

    if (!sessionPtr)
        return makeError();

    CurlSession& session = *sessionPtr;

    CURLcode res = session.httpGet(apiRoot_);

    if (res != CURLE_OK)
    {
        LOG(ERROR) << "GET " << apiRoot_ << " failed: " << curl_easy_strerror(res);
        return makeNetworkError();
    }

    const std::string& responseBody = session.getResponseBodyAsString();

    rapidjson::Document document;

    if (document.Parse(responseBody.c_str()).HasParseError())
    {
        LOG(ERROR) << "Error parsing response";
        return makeError();
    }

    if (hasStringMember(document, kStrAccountsUrl)
        &&  hasStringMember(document, kStrUrl)
        &&  hasStringMember(document, kStrVersion))
    {
        accountsUrl_ = document[kStrAccountsUrl].GetString();

        if (!accountsUrl_.empty()  &&  *accountsUrl_.rbegin() != '/')
            accountsUrl_.push_back('/');
    }
    else
    {
        LOG(ERROR) << "Response JSON must contain three std::string members: "
               << kStrAccountsUrl << ", " << kStrUrl << " and " << kStrVersion;
        return makeError();
    }

    return makeSuccess();
}

// TODO change prefix parse for something more suitable
Status parseAccount(const rapidjson::Value& itemJson, Account& account)
{
    if (!hasIntMember(itemJson, kStrId))
    {
        LOG(ERROR) << "Account JSON must contain integer member " << kStrId;
        return makeError();
    }

    if (!hasStringMember(itemJson, kStrName)
        ||  !hasStringMember(itemJson, kStrUrl)
        ||  !hasStringMember(itemJson, kStrInstrumentsUrl))

    {
        LOG(ERROR) << "Account must have std::string members " << kStrName
               << ", " << kStrUrl << " and " << kStrInstrumentsUrl;
        return makeError();
    }

    // TODO reset account members not set here, may be with account.clear()
    account.id = itemJson[kStrId].GetInt();
    account.name = itemJson[kStrName].GetString();
    account.url = itemJson[kStrUrl].GetString();
    account.instrumentsUrl = itemJson[kStrInstrumentsUrl].GetString();

    return makeSuccess();
}

Status Client::Impl::queryAccountsList(Accounts& accounts)
{
    if (logFlags_ & Client::LOG_INPUT)
        LOG(DEBUG) << __FUNCTION__;

    CurlSessionPtr sessionPtr = createSession();

    if (!sessionPtr)
        return makeError();

    CurlSession& session = *sessionPtr;

    CURLcode res = session.httpGet(accountsUrl_);

    if (res != CURLE_OK)
    {
        LOG(ERROR) << "GET " << accountsUrl_ << " failed: " << curl_easy_strerror(res);
        return makeNetworkError();
    }

    const std::string& responseBody = session.getResponseBodyAsString();

    LOG(DEBUG) << responseBody;

    rapidjson::Document document;

    if (document.Parse(responseBody.c_str()).HasParseError())
    {
        LOG(ERROR) << "Error parsing accounts list JSON";
        return makeError();
    }

    if (!document.IsArray())
    {
        LOG(ERROR) << "Accounts list must be JSON array";
        return makeError();
    }

    accounts.clear();

    for (rapidjson::SizeType i = 0; i < document.Size(); ++i)
    {
        accounts.push_back(Account());
        Status status = parseAccount(document[i], accounts.back());

        if (status.isError())
            return status;
    }

    return makeSuccess();
}


Status Client::Impl::queryAccount(id_t accountId, Account& account)
{
    if (logFlags_ & Client::LOG_INPUT)
        LOG(DEBUG) << __FUNCTION__ << ": accountId = " <<  accountId;

    CurlSessionPtr sessionPtr = createSession();

    if (!sessionPtr)
        return makeError();

    CurlSession& session = *sessionPtr;

    std::string url = getAccountUrl(accountId);

    CURLcode res = session.httpGet(url);

    if (res != CURLE_OK)
    {
        LOG(ERROR) << "GET " << url << " failed: " << curl_easy_strerror(res);
        return makeNetworkError();
    }

    const std::string& responseBody = session.getResponseBodyAsString();

    LOG(DEBUG) << responseBody;

    rapidjson::Document document;

    if (document.Parse(responseBody.c_str()).HasParseError())
    {
        LOG(ERROR) << "Error parsing account JSON";
        return makeError();
    }

    return parseAccount(document, account);
}

Status parseInstrument(const rapidjson::Value& itemJson, Instrument& instrument)
{
    if (!hasIntMember(itemJson, kStrId))
    {
        LOG(ERROR) << "Instrument must have int member " << kStrId;
        return makeError();
    }

    if (!hasStringMember(itemJson, kStrName)
        ||  !hasStringMember(itemJson, kStrInstrumentType))
    {
        LOG(ERROR) << "Instrument must have std::string members " << kStrName
               << " and " << kStrInstrumentType;
        return makeError();
    }

    // TODO reset instrument members, not set here, may be with instrument.clear()
    instrument.id = itemJson[kStrId].GetInt();
    instrument.name = itemJson[kStrName].GetString();
    instrument.type = itemJson[kStrInstrumentType].GetString();

    return makeSuccess();
}

Status Client::Impl::queryInstrumentsList(id_t accountId, Instruments& instruments)
{
    if (logFlags_ & Client::LOG_INPUT)
        LOG(DEBUG) << __FUNCTION__ << ": accountId = " << accountId;

    CurlSessionPtr sessionPtr = createSession();

    if (!sessionPtr)
        return makeError();

    CurlSession& session = *sessionPtr;

    std::string url = getInstrumentsUrl(accountId);

    CURLcode res = session.httpGet(url);

    if (res != CURLE_OK)
    {
        LOG(ERROR) << "GET " << url << " failed: " << curl_easy_strerror(res);
        return makeNetworkError();
    }

    const std::string& responseBody = session.getResponseBodyAsString();

    LOG(DEBUG) << responseBody;

    rapidjson::Document document;

    if (document.Parse(responseBody.c_str()).HasParseError())
    {
        LOG(ERROR) << "Error parsing instruments list JSON";
        return makeError();
    }

    if (!document.IsArray())
    {
        LOG(ERROR) << "Instruments list must be JSON array";
        return makeError();
    }

    instruments.clear();

    for (rapidjson::SizeType i = 0; i < document.Size(); ++i)
    {
        instruments.push_back(Instrument());
        Status status = parseInstrument(document[i], instruments.back());

        if (status.isError())
            return status;
    }

    return makeSuccess();
}

Status Client::Impl::registerInstrument(id_t accountId, const Instrument& instrument)
{
    if (logFlags_ & Client::LOG_INPUT)
    {
        LOG(DEBUG) << __FUNCTION__ << ": accountId = " << accountId
                   << ", instrument{id = " << instrument.id
                   << ", name = " << instrument.name
                   << ", type = " << instrument.type;
    }

    CurlSessionPtr sessionPtr = createSession();

    if (!sessionPtr)
        return makeError();

    CurlSession& session = *sessionPtr;

    std::string url = getInstrumentsUrl(accountId);

    session.addHeader("Content-Type: application/json");

    std::string json = toJsonString(instrument);

    if (logFlags_ & Client::LOG_INPUT_JSON)
    {
        LOG(DEBUG) << __FUNCTION__ << ": instrument JSON: " << json;
    }

    CURLcode res = session.httpPost(url, json);

    if (res != CURLE_OK)
    {
        LOG(ERROR) << "POST " << url << " failed: " << curl_easy_strerror(res);
        return makeNetworkError();
    }

    return makeSuccess();
}

Status Client::Impl::uploadBackground(id_t accountId, id_t instrumentId,
                                        const timestamp_t& timestamp, const Payload& payload)
{
    if (logFlags_ & Client::LOG_INPUT)
    {
        LOG(DEBUG) << __FUNCTION__ << ": accountId = " << accountId
                   << ", instrumentId = " << instrumentId
                   << ", timestamp = " << toIsoTimeString(timestamp)
                   << ", " << toString(payload);
    }

    CurlSessionPtr session = createSession();

    if (!session)
        return makeError();

    CurlSession* cs = session.get();

//    -F "key=BACKGROUND"
//    -F "timestamp=2016-08-17T00:00:00"
//    -F "data=@/path/to/image.png;type=image/png"    cs->addFormField(kStrKey, kStrFLIPBOOK);
    cs->addFormField(kStrKey, kStrBACKGROUND);
    cs->addFormField(kStrTimestamp, toIsoTimeString(timestamp));

    size_t payloadDataSize = payload.dataSize;

    if (payload.data)
        cs->addFormFile(kStrData, payload.data, payload.dataSize, payload.mimeType);
    else
    {
        std::string mimeType = mimeTypeFromFilePath(payload.fileName);
        cs->addFormFile(kStrData, payload.fileName, mimeType);
        payloadDataSize = boost::filesystem::file_size(payload.fileName);
    }

    std::string url = getImagesUrl(accountId, instrumentId);

    CURLcode res = cs->httpPostForm(url);

    if (res != CURLE_OK)
    {
        LOG(ERROR) << "POST " << url << " failed: " << curl_easy_strerror(res);
        return makeNetworkError();
    }

    if (cs->getResponseCode() != 201)
    {
        LOG(ERROR) << "uploadBackground() failed, response code: "
               << cs->getResponseCode() << ", error message: "
               << cs->getErrorMessage() << ", payloadDataSize: " << payloadDataSize;
        return makeError(cs->getResponseCode(), Status::FACILITY_HTTP);
    }

    return makeSuccess();
}

Status Client::Impl::uploadFlipbook(id_t accountId, id_t instrumentId,
                                      const Flipbook& flipbook, const Payload& payload)
{
    if (logFlags_ & Client::LOG_INPUT)
    {
        LOG(DEBUG) << __FUNCTION__ << ": accountId = " << accountId
                   << ", instrumentId = " << instrumentId
                   << ", " << toString(flipbook)
                   << ", " << toString(payload);
    }

    CurlSessionPtr session = createSession();

    if (!session)
        return makeError();

    CurlSession* cs = session.get();

    cs->addFormField(kStrKey, kStrFLIPBOOK);
    cs->addFormField(kStrStartTimestamp, toIsoTimeString(flipbook.startTimestamp));
    cs->addFormField(kStrStopTimestamp, toIsoTimeString(flipbook.stopTimestamp));

    std::string mimeType = payload.data
            ? payload.mimeType
            : mimeTypeFromFilePath(payload.fileName);

    size_t payloadDataSize = payload.dataSize;

    if (payload.data)
        cs->addFormFile(kStrData, payload.data, payload.dataSize, mimeType);
    else
    {
        cs->addFormFile(kStrData, payload.fileName, mimeType);
        payloadDataSize = boost::filesystem::file_size(payload.fileName);
    }

    cs->addFormField(kStrWidth, toString(flipbook.width));
    cs->addFormField(kStrHeight, toString(flipbook.height));
    cs->addFormField(kStrNumberOfFrames, toString(flipbook.numberOfFrames));
    cs->addFormField(kStrContentType, mimeType);

    std::string url = getVideosUrl(accountId, instrumentId);

    CURLcode res = cs->httpPostForm(url);

    if (res != CURLE_OK)
    {
        LOG(ERROR) << "POST " << url << " failed: " << curl_easy_strerror(res);
        return makeNetworkError();
    }

    if (cs->getResponseCode() != 201)
    {
        LOG(ERROR) << "uploadFlipbook() failed, response code: "
               << cs->getResponseCode() << ", error message: "
               << cs->getErrorMessage() << ", payloadDataSize: " << payloadDataSize;;
        return makeError(cs->getResponseCode(), Status::FACILITY_HTTP);
    }

    return makeSuccess();
}

Status Client::Impl::uploadCount(id_t accountId, id_t instrumentId, const Counts& data, bool update)
{
    if (logFlags_ & Client::LOG_INPUT)
    {
        LOG(DEBUG) << __FUNCTION__ << ": accountId = " << accountId
                   << ", instrumentID = " << instrumentId
                   << ", " << toString(data)
                   << ", update = " << update;
    }

    CurlSessionPtr session = createSession();

    if (!session)
        return makeError();

    CurlSession* cs = session.get();

    // -F "key=COUNT"
    cs->addFormField(kStrKey, kStrCOUNT);

    // -F "update=true|false"
    cs->addFormField(kStrUpdate, update ? kStrTrue : kStrFalse);

    // -F "data=<json_as_std::string>;type=application/json"
    std::string json = toJsonString(data);

    if (logFlags_ & Client::LOG_INPUT_JSON)
    {
        LOG(DEBUG) << __FUNCTION__ << ": counts JSON: " << json;
    }

    cs->addFormField(kStrData, json, "application/json");

    std::string url = getTimeSeriesUrl(accountId, instrumentId);

    CURLcode res = cs->httpPostForm(url);

    if (res != CURLE_OK)
    {
        LOG(ERROR) << "POST " << url << " failed: " << curl_easy_strerror(res);
        return makeNetworkError();
    }

    if (cs->getResponseCode() != 201)
    {
        LOG(ERROR) << "uploadCount() failed, response code: "
               << cs->getResponseCode() << ", error message: "
               << cs->getErrorMessage();
        return makeError(cs->getResponseCode(), Status::FACILITY_HTTP);
    }

    return makeSuccess();
}

Status Client::Impl::uploadEvent(id_t accountId, id_t instrumentId,
                                   const timestamp_t& timestamp, const Events& data)
{
    if (logFlags_ & Client::LOG_INPUT)
    {
        LOG(DEBUG) << __FUNCTION__ << ": accountId = " << accountId
                   << ", instrumentId = " << instrumentId
                   << ", timestamp = " << toIsoTimeString(timestamp)
                   << ", " << toString(data);
     }

    CurlSessionPtr session = createSession();

    if (!session)
        return makeError();

    CurlSession* cs = session.get();

//    -F "key=EVENT"
    cs->addFormField(kStrKey, kStrEVENT);

    //    -F "timestamp=2016-08-17T00:00:00"
    cs->addFormField(kStrTimestamp, toIsoTimeString(timestamp));

    //    -F "data=<json_as_std::string>;type=application/json"
    std::string json = toJsonString(data);

    if (logFlags_ & Client::LOG_INPUT_JSON)
    {
        LOG(DEBUG) << __FUNCTION__ << ": events JSON: " << json;
    }

    cs->addFormField(kStrData, json, "application/json");

    std::string url = getTimeSeriesUrl(accountId, instrumentId);

    CURLcode res = cs->httpPostForm(url);

    if (res != CURLE_OK)
    {
        LOG(ERROR) << "POST " << url << " failed: " << curl_easy_strerror(res);
        return makeNetworkError();
    }

    if (cs->getResponseCode() != 201)
    {
        LOG(ERROR) << "uploadEvent() failed, response code: "
               << cs->getResponseCode() << ", error message: "
               << cs->getErrorMessage();
        return makeError(cs->getResponseCode(), Status::FACILITY_HTTP);
    }

    return makeSuccess();
}

Status Client::Impl::uploadObjectStream(id_t accountId, id_t instrumentId,
                                          const ObjectStream& stream, const Payload& payload)
{
    if (logFlags_ & Client::LOG_INPUT)
    {
        LOG(DEBUG) << __FUNCTION__ << ": accountId = " << accountId
                   << ", instrumentId = " << instrumentId
                   << ", " << toString(stream)
                   << ", " << toString(payload);
    }

    CurlSessionPtr session = createSession();

    if (!session)
        return makeError();

    CurlSession* cs = session.get();

    cs->addFormField(kStrKey, kStrOBJECT_STREAM);

    std::string json = toJsonString(stream);

    if (logFlags_ & Client::LOG_INPUT_JSON)
    {
        LOG(DEBUG) << __FUNCTION__ << ": obj stream JSON: " << json;
    }

    cs->addFormField(kStrMeta, json, "application/json");

    if (payload.data)
        cs->addFormFile(kStrData, payload.data, payload.dataSize, payload.mimeType);
    else
    {
        std::string mimeType = mimeTypeFromFilePath(payload.fileName);
        cs->addFormFile(kStrData, payload.fileName.c_str(), mimeType);
    }

    std::string url = getImagesUrl(accountId, instrumentId);

    CURLcode res = cs->httpPostForm(url);

    if (res != CURLE_OK)
    {
        LOG(ERROR) << "POST " << url << " failed: " << curl_easy_strerror(res);
        return makeNetworkError();
    }

    if (cs->getResponseCode() != 201 && cs->getResponseCode() != 200)
    {
        LOG(ERROR) << "uploadObjectStream() failed, response code: "
               << cs->getResponseCode() << ", error message: "
               << cs->getErrorMessage();
        return makeError(cs->getResponseCode(), Status::FACILITY_HTTP);
    }

    return makeSuccess();
}

Status Client::Impl::uploadTrack(id_t accountId, id_t instrumentId,
                                 const timestamp_t& timestamp, const Tracks& data)
{
    if (logFlags_ & Client::LOG_INPUT)
    {
        LOG(DEBUG) << __FUNCTION__ << ": accountId = " << accountId
                   << ", instrumentId = " << instrumentId
                   << ", timestamp = " << toIsoTimeString(timestamp)
                   << ", " << toString(data);
    }

    CurlSessionPtr session = createSession();

    if (!session)
        return makeError();

    CurlSession* cs = session.get();

    // -F "key=TRACK"
    cs->addFormField(kStrKey, kStrTRACK);

    // -F "timestamp=2016-08-17T00:00:00"
    cs->addFormField(kStrTimestamp, toIsoTimeString(timestamp));

    // -F "data=<json_as_std::string>;type=application/json"
    std::string json = toJsonString(data);

    if (logFlags_ & Client::LOG_INPUT_JSON)
    {
        LOG(DEBUG) << __FUNCTION__ << ": tracks JSON: " << json;
    }

    cs->addFormField(kStrData, json, "application/json");

    std::string url = getTimeSeriesUrl(accountId, instrumentId);

    CURLcode res = cs->httpPostForm(url);

    if (res != CURLE_OK)
    {
        LOG(ERROR) << "POST " << url << " failed: " << curl_easy_strerror(res);
        return makeNetworkError();
    }

    if (cs->getResponseCode() != 201)
    {
        LOG(ERROR) << "uploadTrack() failed, response code: "
               << cs->getResponseCode() << ", error message: "
               << cs->getErrorMessage();
        return makeError(cs->getResponseCode(), Status::FACILITY_HTTP);
    }

    return makeSuccess();
}

std::string Client::Impl::getInstrumentsUrl(id_t accountId) const
{
    return accountsUrl_ + toString(accountId) + "/instruments/";
}

std::string Client::Impl::getAccountUrl(id_t accountId) const
{
    return accountsUrl_ + toString(accountId) + '/';
}

std::string Client::Impl::getInstrumentUrl(id_t accountId, id_t instrumentId) const
{
    return getInstrumentsUrl(accountId) + toString(instrumentId) + '/';
}

std::string Client::Impl::getVideosUrl(id_t accountId, id_t instrumentId) const
{
    return getInstrumentUrl(accountId, instrumentId) + "data/videos/";
}

std::string Client::Impl::getImagesUrl(id_t accountId, id_t instrumentId) const
{
    return getInstrumentUrl(accountId, instrumentId) + "data/images/";
}

std::string Client::Impl::getTimeSeriesUrl(id_t accountId, id_t instrumentId) const
{
    return getInstrumentUrl(accountId, instrumentId) + "data/time-series/";
}

CurlSessionPtr Client::Impl::createSession()
{
    CurlSessionPtr sessionPtr = CurlSession::create(token_);

    // apply options stored here, there is no reason to pass them all
    // to CurlSession::create()
    if (sessionPtr)
    {
        CurlSession& session = *sessionPtr;
        session.setConnectionTimeoutMs(connectionTimeoutMs_);
        session.setLowSpeed(lowSpeedTime_, lowSpeedLimit_);
        session.setSslVerifyPeer(sslVerifyPeer_);
    }

    return boost::move(sessionPtr);
}

SdkVersion getSdkVersion()
{
    return SdkVersion(ConnectSDK_VERSION_MAJOR, ConnectSDK_VERSION_MINOR, ConnectSDK_VERSION_REVISION);
}

std::string SdkVersion::toString() const
{
    return boost::str(boost::format("%d.%d.%d") % int(major) % int(minor) % int(revision));
}

Status findCameraByName(Client& client, id_t accountId, const std::string& name,
                        Instrument& cameraInfo)
{
    Instruments instruments;
    Status status = client.queryInstrumentsList(accountId, instruments);

    if (status.isError())
        return status;

    for (size_t i = 0; i < instruments.size(); ++i)
        if (instruments[i].name == name
            && instruments[i].type == kStrCamera)
        {
            cameraInfo = instruments[i];
            return makeSuccess();
        }

    return makeError(Status::NOT_FOUND);
}

Status registerNewCamera(Client& client, id_t accountId, const std::string& name,
                         Instrument& cameraInfo)
{
    Instrument newCamera;
    newCamera.name = name;
    newCamera.type = kStrCamera;

    Status status = client.registerInstrument(accountId, newCamera);

    if (status.isError())
        return status;

    return findCameraByName(client, accountId, name, cameraInfo);
}

}
}
