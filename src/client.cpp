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

    void setProxy(const std::string& proxy)
    {
        proxy_ = proxy;
    }

    void setCaBundlePath(const std::string& caBundlePath)
    {
        caBundlePath_ = caBundlePath;
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
    std::string proxy_;
    std::string caBundlePath_;
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

void Client::setProxy(const std::string& proxy)
{
    impl().setProxy(proxy);
}

bool hasStringMember(const rapidjson::Value& value, const char* name)
{
    return value.HasMember(name)  &&  value[name].IsString();
}

void Client::setCaBundlePath(const std::string& caBundlePath)
{
    impl().setCaBundlePath(caBundlePath);
}

bool hasIntMember(const rapidjson::Value& value, const char* name)
{
    return value.HasMember(name)  &&  value[name].IsInt();
}

Status Client::Impl::init()
{
    // method name as seen by user
    const char* fname = "Client::init()";

    if (logFlags_ & Client::LOG_INPUT)
        LOG(DEBUG) << fname;

    Status rv = makeSuccess();

    do
    {
        CurlSessionPtr sessionPtr = createSession();

        if (!sessionPtr)
        {
            LOG(ERROR) << fname << ": failed to create CURL session";
            rv = makeError();
            break;
        }

        CurlSession& session = *sessionPtr;

        const std::string& url = apiRoot_;
        CURLcode res = session.httpGet(url);

        if (res != CURLE_OK)
        {
            LOG(ERROR) << fname << ": GET " << url << " failed. "
                       << "CURLcode: " << res << ", " << curl_easy_strerror(res);
            rv = makeNetworkError();
            break;
        }

        long responseCode = session.getResponseCode();

        if (responseCode != 200)
        {
            LOG(ERROR) << fname << ": GET " << url << " failed."
                       << " HTTP response code: " << responseCode;
            rv = makeError(responseCode, Status::FACILITY_HTTP);
            break;
        }

        const std::string& responseBody = session.getResponseBodyAsString();

        rapidjson::Document document;

        if (document.Parse(responseBody.c_str()).HasParseError())
        {
            LOG(ERROR) << fname << ": error parsing response: '" << responseBody
                       << "' to GET " << url;
            rv = makeError();
            break;
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
            LOG(ERROR) << fname << ": response JSON must contain three std::string members: "
                       << kStrAccountsUrl << ", " << kStrUrl << " and " << kStrVersion;
            LOG(ERROR) << fname << ": response: '" << responseBody << "' to GET " << url;
            rv = makeError();
            break;
        }

        rv = makeSuccess();
    } while (false);

    if (rv.isError())
        LOG(ERROR) << fname << ": " << rv;

    return rv;
}

Status parseAccountJson(const rapidjson::Value& itemJson, Account& account)
{
    const char* fname = __func__;

    if (!hasIntMember(itemJson, kStrId))
    {
        LOG(ERROR) << fname << ": account JSON must contain integer member " << kStrId;
        return makeError();
    }

    if (!hasStringMember(itemJson, kStrName)
        ||  !hasStringMember(itemJson, kStrUrl)
        ||  !hasStringMember(itemJson, kStrInstrumentsUrl))

    {
        LOG(ERROR) << fname << ": account must have string members " << kStrName
               << ", " << kStrUrl << " and " << kStrInstrumentsUrl;
        return makeError();
    }

    account.clear();
    account.id = itemJson[kStrId].GetInt();
    account.name = itemJson[kStrName].GetString();
    account.url = itemJson[kStrUrl].GetString();
    account.instrumentsUrl = itemJson[kStrInstrumentsUrl].GetString();

    return makeSuccess();
}

Status Client::Impl::queryAccountsList(Accounts& accounts)
{
    const char* fname = "Client::queryAccountsList()";

    if (logFlags_ & Client::LOG_INPUT)
        LOG(DEBUG) << fname;

    Status rv = makeSuccess();

    do
    {
        CurlSessionPtr sessionPtr = createSession();

        if (!sessionPtr)
        {
            LOG(ERROR) << fname << ": failed to create CURL session";
            rv = makeError();
            break;
        }

        CurlSession& session = *sessionPtr;

        const std::string& url = accountsUrl_;
        CURLcode res = session.httpGet(url);

        if (res != CURLE_OK)
        {
            LOG(ERROR) << fname << ": GET " << url << " failed. "
                       << "CURLcode: " << res << ", " << curl_easy_strerror(res);
            rv = makeNetworkError();
            break;
        }

        long responseCode = session.getResponseCode();

        if (responseCode != 200)
        {
            LOG(ERROR) << fname << ": GET " << url << " failed."
                       << " HTTP response code: " << responseCode;
            rv = makeError(responseCode, Status::FACILITY_HTTP);
            break;
        }

        const std::string& responseBody = session.getResponseBodyAsString();

        rapidjson::Document document;

        if (document.Parse(responseBody.c_str()).HasParseError())
        {
            LOG(ERROR) << fname << ": error parsing response: '" << responseBody
                       << "' to GET " << url;
            rv = makeError();
            break;
        }

        if (!document.IsArray())
        {
            LOG(ERROR) << fname << ": accounts list must be JSON array: '"
                       << responseBody << "' to GET " << url;
            rv = makeError();
            break;
        }

        accounts.clear();

        for (rapidjson::SizeType i = 0; i < document.Size(); ++i)
        {
            accounts.push_back(Account());
            rv = parseAccountJson(document[i], accounts.back());

            if (rv.isError())
            {
                // parseAccountJson can't log response body, thus logging it here
                LOG(ERROR) << fname << ": response body: " << responseBody;
                break;
            }
        }
    } while(false);

    if (rv.isError())
        LOG(ERROR) << fname << ": " << rv;

    return rv;
}


Status Client::Impl::queryAccount(id_t accountId, Account& account)
{
    const char* fname = "Client::queryAccount()";

    if (logFlags_ & Client::LOG_INPUT)
        LOG(DEBUG) << fname << ": accountId: " <<  accountId;

    Status rv = makeSuccess();

    do
    {
        CurlSessionPtr sessionPtr = createSession();

        if (!sessionPtr)
        {
            LOG(ERROR) << fname << ": failed to create CURL session";
            rv = makeError();
            break;
        }

        CurlSession& session = *sessionPtr;

        std::string url = getAccountUrl(accountId);
        CURLcode res = session.httpGet(url);

        if (res != CURLE_OK)
        {
            LOG(ERROR) << fname << ": GET " << url << " failed. "
                       << "CURLcode: " << res << ", " << curl_easy_strerror(res);
            rv = makeNetworkError();
            break;
        }

        long responseCode = session.getResponseCode();

        if (responseCode != 200)
        {
            LOG(ERROR) << fname << ": GET " << url << " failed."
                       << " HTTP response code: " << responseCode;
            rv = makeError(responseCode, Status::FACILITY_HTTP);
            break;
        }

        const std::string& responseBody = session.getResponseBodyAsString();

        rapidjson::Document document;

        if (document.Parse(responseBody.c_str()).HasParseError())
        {
            LOG(ERROR) << fname << ": error parsing response: '" << responseBody
                       << "' to GET " << url;
            rv = makeError();
            break;
        }

        rv = parseAccountJson(document, account);

        if (rv.isError())
            LOG(ERROR) << fname << ": response body " << responseBody;

    } while (false);

    if (rv.isError())
        LOG(ERROR) << fname << ": " << rv;

    return rv;
}

Status parseInstrumentJson(const rapidjson::Value& itemJson, Instrument& instrument)
{
    const char* fname = __func__;

    if (!hasIntMember(itemJson, kStrId))
    {
        LOG(ERROR) << fname << ": instrument must have int member " << kStrId;
        return makeError();
    }

    if (!hasStringMember(itemJson, kStrName)
        ||  !hasStringMember(itemJson, kStrInstrumentType))
    {
        LOG(ERROR) << fname << ": instrument must have string members " << kStrName
               << " and " << kStrInstrumentType;
        return makeError();
    }

    instrument.clear();
    instrument.id = itemJson[kStrId].GetInt();
    instrument.name = itemJson[kStrName].GetString();
    instrument.type = itemJson[kStrInstrumentType].GetString();

    return makeSuccess();
}

Status Client::Impl::queryInstrumentsList(id_t accountId, Instruments& instruments)
{
    const char* fname = "Client::queryInstrumentsList()";

    if (logFlags_ & Client::LOG_INPUT)
        LOG(DEBUG) << fname << ": accountId: " << accountId;

    Status rv = makeSuccess();

    do
    {
        CurlSessionPtr sessionPtr = createSession();

        if (!sessionPtr)
        {
            LOG(ERROR) << fname << ": failed to create CURL session";
            rv = makeError();
            break;
        }

        CurlSession& session = *sessionPtr;

        std::string url = getInstrumentsUrl(accountId);

        CURLcode res = session.httpGet(url);

        if (res != CURLE_OK)
        {
            LOG(ERROR) << fname << ": GET " << url << " failed. "
                       << "CURLcode: " << res << ", " << curl_easy_strerror(res);
            rv = makeNetworkError();
            break;
        }

        long responseCode = session.getResponseCode();

        if (responseCode != 200)
        {
            LOG(ERROR) << fname << ": GET " << url << " failed."
                       << " HTTP response code: " << responseCode;
            rv = makeError(responseCode, Status::FACILITY_HTTP);
            break;
        }

        const std::string& responseBody = session.getResponseBodyAsString();

        rapidjson::Document document;

        if (document.Parse(responseBody.c_str()).HasParseError())
        {
            LOG(ERROR) << fname << ": error parsing response: '" << responseBody
                       << "' to GET " << url;
            rv = makeError();
            break;
        }

        if (!document.IsArray())
        {
            LOG(ERROR) << fname << ": JSON array expected, got " << responseBody;
            rv = makeError();
            break;
        }

        instruments.clear();

        for (rapidjson::SizeType i = 0; i < document.Size(); ++i)
        {
            instruments.push_back(Instrument());
            rv = parseInstrumentJson(document[i], instruments.back());

            if (rv.isError())
            {
                LOG(ERROR) << fname << ": response body " << responseBody;
                break;
            }
        }
    } while (false);

    if (rv.isError())
        LOG(ERROR) << fname << ": " << rv;

    return rv;
}

Status Client::Impl::registerInstrument(id_t accountId, const Instrument& instrument)
{
    const char* fname = "Client::registerInstrument()";

    if (logFlags_ & Client::LOG_INPUT)
    {
        LOG(DEBUG) << fname << ": accountId: " << accountId
                   << ", instrument{id: " << instrument.id
                   << ", name: " << instrument.name
                   << ", type: " << instrument.type << "}";
    }

    Status rv = makeSuccess();

    do
    {
        CurlSessionPtr sessionPtr = createSession();

        if (!sessionPtr)
        {
            LOG(ERROR) << fname << ": failed to create CURL session";
            rv = makeError();
            break;
        }

        CurlSession& session = *sessionPtr;
        std::string url = getInstrumentsUrl(accountId);
        session.addHeader("Content-Type: application/json");
        std::string json = toJsonString(instrument);

        if (logFlags_ & Client::LOG_INPUT_JSON)
        {
            LOG(DEBUG) << fname << ": instrument JSON: " << json;
        }

        CURLcode res = session.httpPost(url, json);

        if (res != CURLE_OK)
        {
            LOG(ERROR) << fname << ": POST " << url << " failed. "
                       << "CURLcode: " << res << ", " << curl_easy_strerror(res);
            rv = makeNetworkError();
            break;
        }

        long responseCode = session.getResponseCode();

        if (responseCode != 201)
        {
            LOG(ERROR) << fname << ": POST " << url << " failed."
                       << " HTTP response code: " << responseCode;
            rv = makeError(responseCode, Status::FACILITY_HTTP);
            break;
        }

        rv = makeSuccess();
    } while (false);

    if (rv.isError())
        LOG(ERROR) << fname << ": " << rv;

    return rv;
}

Status Client::Impl::uploadBackground(id_t accountId, id_t instrumentId,
                                        const timestamp_t& timestamp, const Payload& payload)
{
    const char* fname = "Client::uploadBackground()";

    if (logFlags_ & Client::LOG_INPUT)
    {
        LOG(DEBUG) << fname << ": accountId: " << accountId
                   << ", instrumentId: " << instrumentId
                   << ", timestamp: " << toIsoTimeString(timestamp)
                   << ", " << toString(payload);
    }

    Status rv = makeSuccess();

    do
    {
        CurlSessionPtr session = createSession();

        if (!session)
        {
            LOG(ERROR) << fname << ": failed to create CURL session";
            rv = makeError();
            break;
        }

        CurlSession* cs = session.get();

        // -F "key=BACKGROUND"
        // -F "timestamp=2016-08-17T00:00:00"
        // -F "data=@/path/to/image.png;type=image/png"
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
            LOG(ERROR) << fname << ": POST " << url << " failed. "
                       << "CURLcode: " << res << ", " << curl_easy_strerror(res);
            rv = makeNetworkError();
            break;
        }

        long responseCode = cs->getResponseCode();

        if (responseCode != 201)
        {
            LOG(ERROR) << fname << ": POST " << url << " failed."
                       << " HTTP response code: " << responseCode
                       << ", error message: " << cs->getErrorMessage()
                       << ", payloadDataSize: " << payloadDataSize;
            rv = makeError(responseCode, Status::FACILITY_HTTP);
            break;
        }

        rv = makeSuccess();
    } while (false);

    if (rv.isError())
        LOG(ERROR) << fname << ": " << rv;

    return rv;
}

Status Client::Impl::uploadFlipbook(id_t accountId, id_t instrumentId,
                                      const Flipbook& flipbook, const Payload& payload)
{
    const char* fname = "Client::uploadFlipbook()";

    if (logFlags_ & Client::LOG_INPUT)
    {
        LOG(DEBUG) << fname << ": accountId: " << accountId
                   << ", instrumentId: " << instrumentId
                   << ", " << toString(flipbook)
                   << ", " << toString(payload);
    }

    Status rv = makeSuccess();

    do
    {
        CurlSessionPtr session = createSession();

        if (!session)
        {
            LOG(ERROR) << fname << ": failed to create CURL session";
            rv = makeError();
            break;
        }

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
            LOG(ERROR) << fname << ": POST " << url << " failed. "
                       << "CURLcode: " << res << ", " << curl_easy_strerror(res);
            rv = makeNetworkError();
            break;
        }

        long responseCode = cs->getResponseCode();

        if (responseCode != 201)
        {
            LOG(ERROR) << fname << ": POST " << url << " failed. "
                       << " HTTP response code: " << responseCode
                       << ", error message: " << cs->getErrorMessage()
                       << ", payloadDataSize: " << payloadDataSize;;
            rv = makeError(responseCode, Status::FACILITY_HTTP);
            break;
        }

        rv = makeSuccess();
    } while (false);

    if (rv.isError())
        LOG(ERROR) << fname << ": " << rv;

    return rv;
}

Status Client::Impl::uploadCount(id_t accountId, id_t instrumentId, const Counts& data, bool update)
{
    const char* fname = "Client::uploadCount()";

    if (logFlags_ & Client::LOG_INPUT)
    {
        LOG(DEBUG) << fname << ": accountId = " << accountId
                   << ", instrumentID = " << instrumentId
                   << ", " << toString(data)
                   << ", update = " << update;
    }

    Status rv = makeSuccess();

    do
    {
        CurlSessionPtr session = createSession();

        if (!session)
        {
            LOG(ERROR) << fname << ": failed to create CURL session";
            rv = makeError();
            break;
        }

        CurlSession* cs = session.get();

        // -F "key=COUNT"
        cs->addFormField(kStrKey, kStrCOUNT);

        // -F "update=true|false"
        cs->addFormField(kStrUpdate, update ? kStrTrue : kStrFalse);

        // -F "data=<json_as_std::string>;type=application/json"
        std::string json = toJsonString(data);

        if (logFlags_ & Client::LOG_INPUT_JSON)
        {
            LOG(DEBUG) << fname << ": counts JSON: " << json;
        }

        cs->addFormField(kStrData, json, "application/json");

        std::string url = getTimeSeriesUrl(accountId, instrumentId);

        CURLcode res = cs->httpPostForm(url);

        if (res != CURLE_OK)
        {
            LOG(ERROR) << fname << ": POST " << url << " failed. "
                       << "CURLcode: " << res << ", " << curl_easy_strerror(res);
            rv = makeNetworkError();
            break;
        }

        long responseCode = cs->getResponseCode();

        if (responseCode != 201)
        {
            LOG(ERROR) << fname << ": POST " << url << " failed. "
                       << " HTTP response code: " << responseCode
                       << ", error message: " << cs->getErrorMessage();
            rv = makeError(responseCode, Status::FACILITY_HTTP);
            break;
        }

        rv = makeSuccess();
    } while (false);

    if (rv.isError())
        LOG(ERROR) << fname << ": " << rv;

    return rv;
}

Status Client::Impl::uploadEvent(id_t accountId, id_t instrumentId,
                                   const timestamp_t& timestamp, const Events& data)
{
    const char* fname = "Client::uploadEvent()";

    if (logFlags_ & Client::LOG_INPUT)
    {
        LOG(DEBUG) << fname << ": accountId: " << accountId
                   << ", instrumentId: " << instrumentId
                   << ", timestamp: " << toIsoTimeString(timestamp)
                   << ", " << toString(data);
    }

    Status rv = makeSuccess();

    do
    {
        CurlSessionPtr session = createSession();

        if (!session)
        {
            LOG(ERROR) << fname << ": failed to create CURL session";
            rv = makeError();
            break;
        }

        CurlSession* cs = session.get();

        // -F "key=EVENT"
        cs->addFormField(kStrKey, kStrEVENT);

        // -F "timestamp=2016-08-17T00:00:00"
        cs->addFormField(kStrTimestamp, toIsoTimeString(timestamp));

        // -F "data=<json_as_std::string>;type=application/json"
        std::string json = toJsonString(data);

        if (logFlags_ & Client::LOG_INPUT_JSON)
        {
            LOG(DEBUG) << fname << ": events JSON: " << json;
        }

        cs->addFormField(kStrData, json, "application/json");

        std::string url = getTimeSeriesUrl(accountId, instrumentId);

        CURLcode res = cs->httpPostForm(url);

        if (res != CURLE_OK)
        {
            LOG(ERROR) << fname << ": POST " << url << " failed. "
                       << "CURLcode: " << res << ", " << curl_easy_strerror(res);
            rv = makeNetworkError();
            break;
        }

        long responseCode = cs->getResponseCode();

        if (responseCode != 201)
        {
            LOG(ERROR) << fname << ": POST " << url << " failed. "
                       << " HTTP response code: " << responseCode
                       << ", error message: " << cs->getErrorMessage();
            rv = makeError(cs->getResponseCode(), Status::FACILITY_HTTP);
            break;
        }

        rv = makeSuccess();
    } while (false);

    if (rv.isError())
        LOG(ERROR) << fname << ": " << rv;

    return rv;
}

Status Client::Impl::uploadObjectStream(id_t accountId, id_t instrumentId,
                                          const ObjectStream& stream, const Payload& payload)
{
    const char* fname = "Client::uploadObjectStream()";

    if (logFlags_ & Client::LOG_INPUT)
    {
        LOG(DEBUG) << fname << ": accountId: " << accountId
                   << ", instrumentId: " << instrumentId
                   << ", " << toString(stream)
                   << ", " << toString(payload);
    }

    Status rv = makeSuccess();

    do
    {
        CurlSessionPtr session = createSession();

        if (!session)
        {
            LOG(ERROR) << fname << ": failed to create CURL session";
            rv = makeError();
            break;
        }

        CurlSession* cs = session.get();

        cs->addFormField(kStrKey, kStrOBJECT_STREAM);

        std::string json = toJsonString(stream);

        if (logFlags_ & Client::LOG_INPUT_JSON)
        {
            LOG(DEBUG) << fname << ": obj stream JSON: " << json;
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
            LOG(ERROR) << fname << ": POST " << url << " failed. "
                       << "CURLcode: " << res << ", " << curl_easy_strerror(res);
            rv = makeNetworkError();
            break;
        }

        long responseCode = cs->getResponseCode();

        if (responseCode != 201 && responseCode != 200)
        {
            LOG(ERROR) << fname << ": POST " << url << " failed. "
                       << " HTTP response code: " << responseCode
                       << ", error message: " << cs->getErrorMessage();
            rv = makeError(responseCode, Status::FACILITY_HTTP);
            break;
        }

        rv = makeSuccess();
    } while (false);

    if (rv.isError())
        LOG(ERROR) << fname << ": " << rv;

    return rv;
}

Status Client::Impl::uploadTrack(id_t accountId, id_t instrumentId,
                                 const timestamp_t& timestamp, const Tracks& data)
{
    const char* fname = "Client::uploadTrack()";

    if (logFlags_ & Client::LOG_INPUT)
    {
        LOG(DEBUG) << fname << ": accountId: " << accountId
                   << ", instrumentId: " << instrumentId
                   << ", timestamp: " << toIsoTimeString(timestamp)
                   << ", " << toString(data);
    }

    Status rv = makeSuccess();

    do
    {
        CurlSessionPtr session = createSession();

        if (!session)
        {
            LOG(ERROR) << fname << ": failed to create CURL session";
            rv = makeError();
            break;
        }

        CurlSession* cs = session.get();

        // -F "key=TRACK"
        cs->addFormField(kStrKey, kStrTRACK);

        // -F "timestamp=2016-08-17T00:00:00"
        cs->addFormField(kStrTimestamp, toIsoTimeString(timestamp));

        // -F "data=<json_as_std::string>;type=application/json"
        std::string json = toJsonString(data);

        if (logFlags_ & Client::LOG_INPUT_JSON)
            LOG(DEBUG) << fname << ": tracks JSON: " << json;

        cs->addFormField(kStrData, json, "application/json");

        std::string url = getTimeSeriesUrl(accountId, instrumentId);

        CURLcode res = cs->httpPostForm(url);

        if (res != CURLE_OK)
        {
            LOG(ERROR) << fname << ": POST " << url << " failed. "
                       << "CURLcode: " << res << ", " << curl_easy_strerror(res);
            // no need to log POST parameters here
            rv = makeNetworkError();
            break;
        }

        if (cs->getResponseCode() != 201)
        {
            LOG(ERROR) << fname << ": POST " << url << " failed. "
                       << cs->getResponseCode() << ", error message: "
                       << cs->getErrorMessage();
            rv = makeError(cs->getResponseCode(), Status::FACILITY_HTTP);
            break;
        }
    } while (false);

    if (rv.isError())
        LOG(ERROR) << fname << ": " << rv;

    return rv;
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

    // apply stored options here, there is no reason to pass them all
    // to CurlSession::create()
    if (sessionPtr)
    {
        CurlSession& session = *sessionPtr;
        session.setConnectionTimeoutMs(connectionTimeoutMs_);
        session.setLowSpeed(lowSpeedTime_, lowSpeedLimit_);
        session.setSslVerifyPeer(sslVerifyPeer_);
        session.setProxy(proxy_);
        session.setCaBundlePath(caBundlePath_);
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
