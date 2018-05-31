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

namespace prism {
namespace connect {

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
    Status queryFeedsList(id_t accountId, Feeds& feeds);
    Status registerFeed(id_t accountId, const Feed& feed);

    Status uploadBackground(id_t accountId, id_t feedId, const Background& background, const Payload& payload);
    Status uploadFlipbook(id_t accountId, id_t feedId, const Flipbook& flipbook, const Payload& payload);
    Status uploadObjectSnapshot(id_t accountId, id_t feedId, const ObjectSnapshot& snapshot, const Payload& payload);
    Status uploadTrack(id_t accountId, id_t feedId, const Track& track);
    Status uploadTimeSeries(id_t accountId, id_t feedId, const TimeSeries& series);

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
    std::string getFeedsUrl(id_t accountId) const;
    std::string getAccountUrl(id_t accountId) const;
    std::string getFeedUrl(id_t accountId, id_t feedId) const;
    std::string getArtifactsUrl(id_t accountId, id_t feedId) const;

    CurlSessionPtr createSession();

    Status parseAccountJson(const rapidjson::Value& itemJson, Account& account);

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

Status Client::queryApiState(std::string& /*accountsUrl*/, std::string& /*apiVersion*/)
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

Status Client::queryFeedsList(id_t accountId, Feeds &feeds)
{
    return impl().queryFeedsList(accountId, feeds);
}

Status Client::registerFeed(id_t accountId, const Feed& feed)
{
    return impl().registerFeed(accountId, feed);
}

Status Client::uploadBackground(id_t accountId, id_t feedId,
                                const Background& background, const Payload& payload)
{
    return impl().uploadBackground(accountId, feedId, background, payload);
}

Status Client::uploadFlipbook(id_t accountId, id_t feedId,
                              const Flipbook& flipbook, const Payload& payload)
{
    return impl().uploadFlipbook(accountId, feedId, flipbook, payload);
}

Status Client::uploadObjectSnapshot(id_t accountId, id_t feedId,
                                    const ObjectSnapshot& snapshot, const Payload& payload)
{
    return impl().uploadObjectSnapshot(accountId, feedId, snapshot, payload);
}

Status Client::uploadTrack(id_t accountId, id_t feedId, const Track& track)
{
    return impl().uploadTrack(accountId, feedId, track);
}

Status Client::uploadTimeSeries(id_t accountId, id_t feedId, const TimeSeries& series)
{
    return impl().uploadTimeSeries(accountId, feedId, series);
}

void Client::setLogFlags(int logFlags)
{
    impl().setLogFlags(logFlags);
}

void Client::setProxy(const std::string& proxy)
{
    impl().setProxy(proxy);
}

void Client::setCaBundlePath(const std::string& caBundlePath)
{
    impl().setCaBundlePath(caBundlePath);
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
        const std::string& responseBody = session.getResponseBodyAsString();

        if (responseCode != 200)
        {
            LOG(ERROR) << fname << ": GET " << url << " failed."
                       << " HTTP response code: " << responseCode
                       << ", body: " << responseBody;
            rv = makeError(responseCode, Status::FACILITY_HTTP);
            break;
        }

        if (logFlags_ & Client::LOG_RESPONSE)
            LOG(DEBUG) << fname << ": response: " << responseBody;

        rapidjson::Document document;

        if (document.Parse(responseBody.c_str()).HasParseError())
        {
            LOG(ERROR) << fname << ": error parsing response: '" << responseBody
                       << "' to GET " << url;
            rv = makeError();
            break;
        }

        if (!hasStringMember(document, kStrVersion))
            LOG(WARNING) << fname << ": response JSON doesn't have " << kStrVersion;

        if (!hasStringMember(document, kStrUrl))
            LOG(WARNING) << fname << ": response JSON doesn't have " << kStrUrl;

        if (hasStringMember(document, kStrAccountsUrl))
        {
            accountsUrl_ = document[kStrAccountsUrl].GetString();

            if (!accountsUrl_.empty()  &&  *accountsUrl_.rbegin() != '/')
                accountsUrl_.push_back('/');
        }
        else
        {
            accountsUrl_ = apiRoot_;

            if (!accountsUrl_.empty()  &&  *accountsUrl_.rbegin() != '/')
                accountsUrl_.push_back('/');

            accountsUrl_.append("accounts/");

            LOG(WARNING) << fname << ": response JSON doesn't have " << kStrAccountsUrl
                         << ", using " << accountsUrl_ << " as accounts URL";
        }

        rv = makeSuccess();
    } while (false);

    if (rv.isError())
        LOG(ERROR) << fname << ": " << rv;

    return rv;
}

Status Client::Impl::parseAccountJson(const rapidjson::Value& itemJson,
                                      Account& account)
{
    const char* fname = "Client::parseAccountsJson()";

    account.clear();

    if (!hasIntMember(itemJson, kStrId))
    {
        LOG(ERROR) << fname << ": account JSON must contain integer member " << kStrId;
        return makeError();
    }

    account.id = itemJson[kStrId].GetInt();

    if (hasStringMember(itemJson, kStrName))
        account.name = itemJson[kStrName].GetString();
    else
    {
        LOG(WARNING) << fname << ": account JSON for id " << account.id
                     << " doesn't have " << kStrName
                     << ". Using empty name";
    }

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
            LOG(ERROR) << fname << ": GET \"" << url << "\" failed. "
                       << "CURLcode: " << res << ", " << curl_easy_strerror(res);
            rv = makeNetworkError();
            break;
        }

        long responseCode = session.getResponseCode();
        const std::string& responseBody = session.getResponseBodyAsString();

        if (responseCode != 200)
        {
            LOG(ERROR) << fname << ": GET " << url << " failed."
                       << " HTTP response code: " << responseCode
                       << ", body: " << responseBody;
            rv = makeError(responseCode, Status::FACILITY_HTTP);
            break;
        }

        if (logFlags_ & Client::LOG_RESPONSE)
            LOG(DEBUG) << fname << ": response: " << responseBody;

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
        const std::string& responseBody = session.getResponseBodyAsString();

        if (responseCode != 200)
        {
            LOG(ERROR) << fname << ": GET " << url << " failed."
                       << " HTTP response code: " << responseCode
                       << ", body: " << responseBody;
            rv = makeError(responseCode, Status::FACILITY_HTTP);
            break;
        }

        if (logFlags_ & Client::LOG_RESPONSE)
            LOG(DEBUG) << fname << ": response: " << responseBody;

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

Status parseFeedJson(const rapidjson::Value& itemJson, Feed& feed)
{
    const char* fname = __func__;

    feed.clear();

    if (!hasIntMember(itemJson, kStrId))
    {
        LOG(ERROR) << fname << ": feed must have int member " << kStrId;
        return makeError();
    }

    feed.id = itemJson[kStrId].GetInt();

    if (!hasStringMember(itemJson, kStrName))
    {
        LOG(ERROR) << fname << ": feed must have string members " << kStrName;
        return makeError();
    }

    feed.name = itemJson[kStrName].GetString();

    return makeSuccess();
}

Status Client::Impl::queryFeedsList(id_t accountId, Feeds& feeds)
{
    const char* fname = "Client::queryFeedsList()";

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

        std::string url = getFeedsUrl(accountId);

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

        feeds.clear();

        for (rapidjson::SizeType i = 0; i < document.Size(); ++i)
        {
            feeds.push_back(Feed());
            rv = parseFeedJson(document[i], feeds.back());

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

Status Client::Impl::registerFeed(id_t accountId, const Feed& feed)
{
    const char* fname = "Client::registerFeed()";

    if (logFlags_ & Client::LOG_INPUT)
    {
        LOG(DEBUG) << fname << ": accountId: " << accountId
                   << ", feed{id: " << feed.id
                   << ", name: " << feed.name
                   << "}";
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
        std::string url = getFeedsUrl(accountId);
        session.addHeader("Content-Type: application/json");
        std::string json = toJsonString(feed);

        if (logFlags_ & Client::LOG_INPUT_JSON)
        {
            LOG(DEBUG) << fname << ": feed JSON: " << json;
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

Status Client::Impl::uploadBackground(id_t accountId, id_t feedId,
                                      const Background& background, const Payload& payload)
{
    const char* fname = "Client::uploadBackground()";

    if (logFlags_ & Client::LOG_INPUT)
    {
        LOG(DEBUG) << fname << ": accountId: " << accountId
                   << ", feedId: " << feedId
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
        //cs->addFormField(kStrTimestamp, toIsoTimeString(timestamp));

        std::string mimeType = payload.data
                ? payload.mimeType
                : mimeTypeFromFilePath(payload.fileName);

        std::string json = toJsonString(background, mimeType);

        if (logFlags_ & Client::LOG_INPUT_JSON)
        {
            LOG(DEBUG) << fname << ": obj stream JSON: " << json;
        }

        cs->addFormField(kStrMeta, json, "application/json");

//        std::string mimeType = payload.data
//                ? payload.mimeType
//                : mimeTypeFromFilePath(payload.fileName);

        size_t payloadDataSize = payload.dataSize;

        if (payload.data)
            cs->addFormFile(kStrData, payload.data, payloadDataSize, mimeType);
        else
        {
            cs->addFormFile(kStrData, payload.fileName, mimeType);
            payloadDataSize = boost::filesystem::file_size(payload.fileName);
        }

        std::string url = getArtifactsUrl(accountId, feedId);

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

Status Client::Impl::uploadFlipbook(id_t accountId, id_t feedId,
                                    const Flipbook& flipbook, const Payload& payload)
{
    const char* fname = "Client::uploadFlipbook()";

    if (logFlags_ & Client::LOG_INPUT)
    {
        LOG(DEBUG) << fname << ": accountId: " << accountId
                   << ", feedId: " << feedId
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

        std::string mimeType = payload.data
                ? payload.mimeType
                : mimeTypeFromFilePath(payload.fileName);

        std::string json = toJsonString(flipbook, mimeType);

        if (logFlags_ & Client::LOG_INPUT_JSON)
        {
            LOG(DEBUG) << fname << ": obj stream JSON: " << json;
        }

        cs->addFormField(kStrMeta, json, "application/json");

//        cs->addFormField(kStrBegin, toIsoTimeString(flipbook.begin));
//        cs->addFormField(kStrEnd, toIsoTimeString(flipbook.end));

//        std::string mimeType = payload.data
//                ? payload.mimeType
//                : mimeTypeFromFilePath(payload.fileName);

        size_t payloadDataSize = payload.dataSize;

        if (payload.data)
            cs->addFormFile(kStrData, payload.data, payloadDataSize, mimeType);
        else
        {
            cs->addFormFile(kStrData, payload.fileName, mimeType);
            payloadDataSize = boost::filesystem::file_size(payload.fileName);
        }

//        cs->addFormField(kStrWidth, toString(flipbook.width));
//        cs->addFormField(kStrHeight, toString(flipbook.height));
//        cs->addFormField(kStrNumberOfFrames, toString(flipbook.numberOfFrames));
//        cs->addFormField(kStrContentType, mimeType);

        std::string url = getArtifactsUrl(accountId, feedId);

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

Status Client::Impl::uploadObjectSnapshot(id_t accountId, id_t feedId,
                                          const ObjectSnapshot& snapshot, const Payload& payload)
{
    const char* fname = "Client::uploadObjectSnapshot()";

    if (logFlags_ & Client::LOG_INPUT)
    {
        LOG(DEBUG) << fname << ": accountId: " << accountId
                   << ", feedId: " << feedId
                   << ", " << toString(snapshot)
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

        std::string mimeType = payload.data
                ? payload.mimeType
                : mimeTypeFromFilePath(payload.fileName);

        std::string json = toJsonString(snapshot, mimeType);

        if (logFlags_ & Client::LOG_INPUT_JSON)
        {
            LOG(DEBUG) << fname << ": obj snapshot JSON: " << json;
        }

        cs->addFormField(kStrMeta, json, "application/json");

        if (payload.data)
            cs->addFormFile(kStrData, payload.data, payload.dataSize, mimeType);
        else
        {
            cs->addFormFile(kStrData, payload.fileName.c_str(), mimeType);
        }

        std::string url = getArtifactsUrl(accountId, feedId);

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

Status Client::Impl::uploadTrack(id_t accountId, id_t feedId, const Track& track)
{
    const char* fname = "Client::uploadTrack()";

    if (logFlags_ & Client::LOG_INPUT)
    {
        LOG(DEBUG) << fname << ": accountId: " << accountId
                   << ", feedId: " << feedId
                   << ", " << toString(track);
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

        // -F "meta=<json_as_std::string>;type=application/json"
        std::string json = toJsonString(track, "media/json");

        if (logFlags_ & Client::LOG_INPUT_JSON)
            LOG(DEBUG) << fname << ": track JSON: " << json;

        cs->addFormField(kStrMeta, json, "application/json");

        std::string url = getArtifactsUrl(accountId, feedId);

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

Status Client::Impl::uploadTimeSeries(id_t accountId, id_t feedId, const TimeSeries& series)
{
    const char* fname = "Client::uploadTimeSeries()";

    if (logFlags_ & Client::LOG_INPUT)
    {
        LOG(DEBUG) << fname << ": accountId = " << accountId
                   << ", feedId = " << feedId
                   << ", " << toString(series);
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

        // -F "key=TSERIE"
        cs->addFormField(kStrKey, kStrTSERIE);

        // -F "meta=<json_as_std::string>;type=application/json"
        std::string json = toJsonString(series, "media/json");

        if (logFlags_ & Client::LOG_INPUT_JSON)
        {
            LOG(DEBUG) << fname << ": series JSON: " << json;
        }

        cs->addFormField(kStrMeta, json, "application/json");

        std::string url = getArtifactsUrl(accountId, feedId);

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


std::string Client::Impl::getFeedsUrl(id_t accountId) const
{
    return accountsUrl_ + toString(accountId) + "/feeds/";
}

std::string Client::Impl::getAccountUrl(id_t accountId) const
{
    return accountsUrl_ + toString(accountId) + '/';
}

std::string Client::Impl::getFeedUrl(id_t accountId, id_t feedId) const
{
    return getFeedsUrl(accountId) + toString(feedId) + '/';
}

std::string Client::Impl::getArtifactsUrl(id_t accountId, id_t feedId) const
{
    return getFeedUrl(accountId, feedId) + "data/artifacts/";
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
        if (!caBundlePath_.empty())
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
                        Feed& cameraInfo)
{
    Feeds feeds;
    Status status = client.queryFeedsList(accountId, feeds);

    if (status.isError())
        return status;

    for (size_t i = 0; i < feeds.size(); ++i)
        if (feeds[i].name == name)
        {
            cameraInfo = feeds[i];
            return makeSuccess();
        }

    return makeError(Status::NOT_FOUND);
}

Status registerNewCamera(Client& client, id_t accountId, const std::string& name,
                         Feed& cameraInfo)
{
    Feed newCamera;
    newCamera.name = name;

    Status status = client.registerFeed(accountId, newCamera);

    if (status.isError())
        return status;

    return findCameraByName(client, accountId, name, cameraInfo);
}

} // connect
} // prism
