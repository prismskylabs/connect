/*
 * Copyright (C) 2016-2017 Prism Skylabs
 */
#include "client.h"
#include "const-strings.h"
#include "curl-session.h"
#include "util.h"
#include "easylogging++.h"
#include "rapidjson/document.h"

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
    {
    }

    status_t init();

    status_t queryAccountsList(Accounts& accounts);
    status_t queryAccount(id_t accountId, Account &account);
    status_t queryInstrumentsList(id_t accountId, Instruments& instruments);
    status_t registerInstrument(id_t accountId, const Instrument& instrument);

    status_t uploadBackground(id_t accountId, id_t instrumentId,
                              const timestamp_t& timestamp, const Payload& payload);

    status_t uploadFlipbook(id_t accountId, id_t instrumentId,
                            const Flipbook& flipbook, const Payload& payload);

    status_t uploadEvent(id_t accountId, id_t instrumentId,
                         const timestamp_t& timestamp, const Events& data);

    status_t uploadObjectStream(id_t accountId, id_t instrumentId,
                                const ObjectStream& stream, const Payload& payload);

private:
    std::string getInstrumentsUrl(id_t accountId) const;
    std::string getAccountUrl(id_t accountId) const;
    std::string getInstrumentUrl(id_t accountId, id_t instrumentId) const;
    std::string getVideosUrl(id_t accountId, id_t instrumentId) const;
    std::string getImagesUrl(id_t accountId, id_t instrumentId) const;
    std::string getTimeSeriesUrl(id_t accountId, id_t instrumentId) const;

    std::string apiRoot_;
    std::string token_;

    std::string accountsUrl_;
};

Client::Client(const std::string& apiRoot, const std::string& token)
    : pImpl_(new Impl(apiRoot, token))
{
}

Client::~Client()
{
    delete pImpl_;
}

status_t Client::init()
{
    return pImpl_->init();
}

status_t Client::queryApiState(std::string& accountsUrl, std::string& apiVersion)
{
    return STATUS_ERROR;
}

status_t Client::queryAccountsList(Accounts& accounts)
{
    return pImpl_->queryAccountsList(accounts);
}

status_t Client::queryAccount(id_t accountId, Account &account)
{
    return pImpl_->queryAccount(accountId, account);
}

status_t Client::queryInstrumentsList(id_t accountId, Instruments &instruments)
{
    return pImpl_->queryInstrumentsList(accountId, instruments);
}

status_t Client::registerInstrument(id_t accountId, const Instrument& instrument)
{
    return pImpl_->registerInstrument(accountId, instrument);
}

status_t Client::uploadBackground(id_t accountId, id_t instrumentId,
                                  const timestamp_t& timestamp, const Payload& payload)
{
    return pImpl_->uploadBackground(accountId, instrumentId, timestamp, payload);
}

status_t Client::uploadObjectStream(id_t accountId, id_t instrumentId,
                                    const ObjectStream& stream, const Payload& payload)
{
    return pImpl_->uploadObjectStream(accountId, instrumentId, stream, payload);
}

status_t Client::uploadFlipbook(id_t accountId, id_t instrumentId,
                                const Flipbook& flipbook, const Payload& payload)
{
    return pImpl_->uploadFlipbook(accountId, instrumentId, flipbook, payload);
}

status_t Client::uploadEvent(id_t accountId, id_t instrumentId,
                             const timestamp_t& timestamp, const Events& data)
{
    return pImpl_->uploadEvent(accountId, instrumentId, timestamp, data);
}

// TODO move to utils?
bool hasStringMember(const rapidjson::Value& value, const char* name)
{
    return value.HasMember(name)  &&  value[name].IsString();
}

bool hasIntMember(const rapidjson::Value& value, const char* name)
{
    return value.HasMember(name)  &&  value[name].IsInt();
}

status_t Client::Impl::init()
{
    CurlSessionPtr session = CurlSession::create(token_);

    if (!session)
        return STATUS_ERROR;

    CURLcode res = session->httpGet(apiRoot_);

    if (res != CURLE_OK)
    {
        LOG(ERROR) << "GET " << apiRoot_ << " failed: " << curl_easy_strerror(res);
        return STATUS_ERROR;
    }

    const std::string& responseBody = session->getResponseBodyAsString();

    rapidjson::Document document;

    if (document.Parse(responseBody.c_str()).HasParseError())
    {
        LOG(ERROR) << "Error parsing response";
        return STATUS_ERROR;
    }

    if (hasStringMember(document, kStrAccountsUrl)
        &&  hasStringMember(document, kStrUrl)
        &&  hasStringMember(document, kStrVersion))
    {
        accountsUrl_ = document[kStrAccountsUrl].GetString();

        if (!accountsUrl_.empty()  &&  accountsUrl_.back() != '/')
            accountsUrl_.push_back('/');
    }
    else
    {
        LOG(ERROR) << "Response JSON must contain three std::string members: "
               << kStrAccountsUrl << ", " << kStrUrl << " and " << kStrVersion;
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

// TODO change prefix parse for something more suitable
status_t parseAccount(const rapidjson::Value& itemJson, Account& account)
{
    if (!hasIntMember(itemJson, kStrId))
    {
        LOG(ERROR) << "Account JSON must contain integer member " << kStrId;
        return STATUS_ERROR;
    }

    if (!hasStringMember(itemJson, kStrName)
        ||  !hasStringMember(itemJson, kStrUrl)
        ||  !hasStringMember(itemJson, kStrInstrumentsUrl))

    {
        LOG(ERROR) << "Account must have std::string members " << kStrName
               << ", " << kStrUrl << " and " << kStrInstrumentsUrl;
        return STATUS_ERROR;
    }

    // TODO reset account members not set here, may be with account.clear()
    account.id = itemJson[kStrId].GetInt();
    account.name = itemJson[kStrName].GetString();
    account.url = itemJson[kStrUrl].GetString();
    account.instrumentsUrl = itemJson[kStrInstrumentsUrl].GetString();

    return STATUS_OK;
}

status_t Client::Impl::queryAccountsList(Accounts& accounts)
{
    CurlSessionPtr session = CurlSession::create(token_);

    if (!session)
        return STATUS_ERROR;

    CURLcode res = session->httpGet(accountsUrl_);

    if (res != CURLE_OK)
    {
        LOG(ERROR) << "GET " << accountsUrl_ << " failed: " << curl_easy_strerror(res);
        return STATUS_ERROR;
    }

    const std::string& responseBody = session->getResponseBodyAsString();

    LOG(INFO) << responseBody;

    rapidjson::Document document;

    if (document.Parse(responseBody.c_str()).HasParseError())
    {
        LOG(ERROR) << "Error parsing accounts list JSON";
        return STATUS_ERROR;
    }

    if (!document.IsArray())
    {
        LOG(ERROR) << "Accounts list must be JSON array";
        return STATUS_ERROR;
    }

    accounts.clear();

    for (rapidjson::SizeType i = 0; i < document.Size(); ++i)
    {
        accounts.push_back(Account());
        status_t status = parseAccount(document[i], accounts.back());

        if (status != STATUS_OK)
            return status;
    }

    return STATUS_OK;
}


status_t Client::Impl::queryAccount(id_t accountId, Account& account)
{
    CurlSessionPtr session = CurlSession::create(token_);

    if (!session)
        return STATUS_ERROR;

    std::string url = getAccountUrl(accountId);

    CURLcode res = session->httpGet(url);

    if (res != CURLE_OK)
    {
        LOG(ERROR) << "GET " << url << " failed: " << curl_easy_strerror(res);
        return STATUS_ERROR;
    }

    const std::string& responseBody = session->getResponseBodyAsString();

    LOG(INFO) << responseBody;

    rapidjson::Document document;

    if (document.Parse(responseBody.c_str()).HasParseError())
    {
        LOG(ERROR) << "Error parsing account JSON";
        return STATUS_ERROR;
    }

    return parseAccount(document, account);
}

status_t parseInstrument(const rapidjson::Value& itemJson, Instrument& instrument)
{
    if (!hasIntMember(itemJson, kStrId))
    {
        LOG(ERROR) << "Instrument must have int member " << kStrId;
        return STATUS_ERROR;
    }

    if (!hasStringMember(itemJson, kStrName)
        ||  !hasStringMember(itemJson, kStrInstrumentType))
    {
        LOG(ERROR) << "Instrument must have std::string members " << kStrName
               << " and " << kStrInstrumentType;
        return STATUS_ERROR;
    }

    // TODO reset instrument members, not set here, may be with instrument.clear()
    instrument.id = itemJson[kStrId].GetInt();
    instrument.name = itemJson[kStrName].GetString();
    instrument.type = itemJson[kStrInstrumentType].GetString();

    return STATUS_OK;
}

status_t Client::Impl::queryInstrumentsList(id_t accountId, Instruments& instruments)
{
    CurlSessionPtr session = CurlSession::create(token_);

    if (!session)
        return STATUS_ERROR;

    std::string url = getInstrumentsUrl(accountId);

    CURLcode res = session->httpGet(url);

    if (res != CURLE_OK)
    {
        LOG(ERROR) << "GET " << url << " failed: " << curl_easy_strerror(res);
        return STATUS_ERROR;
    }

    const std::string& responseBody = session->getResponseBodyAsString();

    LOG(INFO) << responseBody;

    rapidjson::Document document;

    if (document.Parse(responseBody.c_str()).HasParseError())
    {
        LOG(ERROR) << "Error parsing instruments list JSON";
        return STATUS_ERROR;
    }

    if (!document.IsArray())
    {
        LOG(ERROR) << "Instruments list must be JSON array";
        return STATUS_ERROR;
    }

    instruments.clear();

    for (rapidjson::SizeType i = 0; i < document.Size(); ++i)
    {
        instruments.push_back(Instrument());
        status_t status = parseInstrument(document[i], instruments.back());

        if (status != STATUS_OK)
            return status;
    }

    return STATUS_OK;
}

status_t Client::Impl::registerInstrument(id_t accountId, const Instrument& instrument)
{
    CurlSessionPtr session = CurlSession::create(token_);

    if (!session)
        return STATUS_ERROR;

    std::string url = getInstrumentsUrl(accountId);

    session->addHeader("Content-Type: application/json");

    CURLcode res = session->httpPost(url, toJsonString(instrument));

    if (res != CURLE_OK)
    {
        LOG(ERROR) << "POST " << url << " failed: " << curl_easy_strerror(res);
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

status_t Client::Impl::uploadBackground(id_t accountId, id_t instrumentId,
                                        const timestamp_t& timestamp, const Payload& payload)
{
    CurlSessionPtr session = CurlSession::create(token_);

    if (!session)
        return STATUS_ERROR;

    CurlSession* cs = session.get();

//    -F "key=BACKGROUND"
//    -F "timestamp=2016-08-17T00:00:00"
//    -F "data=@/path/to/image.png;type=image/png"    cs->addFormField(kStrKey, kStrFLIPBOOK);
    cs->addFormField(kStrKey, kStrBACKGROUND);
    cs->addFormField(kStrTimestamp, toIsoTimeString(timestamp));

    if (payload.data)
        cs->addFormFile(kStrData, payload.data, payload.dataSize, payload.mimeType);
    else
    {
        std::string mimeType = mimeTypeFromFilePath(payload.fileName);
        cs->addFormFile(kStrData, payload.fileName, mimeType);
    }

    std::string url = getImagesUrl(accountId, instrumentId);

    CURLcode res = cs->httpPostForm(url);

    if (res != CURLE_OK)
    {
        LOG(ERROR) << "POST " << url << " failed: " << curl_easy_strerror(res);
        return STATUS_ERROR;
    }

    if (cs->getResponseCode() != 201)
    {
        LOG(ERROR) << "uploadBackground() failed, response code: "
               << cs->getResponseCode() << ", error message: "
               << cs->getErrorMessage();
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

status_t Client::Impl::uploadFlipbook(id_t accountId, id_t instrumentId,
                                      const Flipbook& flipbook, const Payload& payload)
{
    CurlSessionPtr session = CurlSession::create(token_);

    if (!session)
        return STATUS_ERROR;

    CurlSession* cs = session.get();

    cs->addFormField(kStrKey, kStrFLIPBOOK);
    cs->addFormField(kStrStartTimestamp, toIsoTimeString(flipbook.startTimestamp));
    cs->addFormField(kStrStopTimestamp, toIsoTimeString(flipbook.stopTimestamp));

    std::string mimeType = payload.data
            ? payload.mimeType
            : mimeTypeFromFilePath(payload.fileName);

    if (payload.data)
        cs->addFormFile(kStrData, payload.data, payload.dataSize, mimeType);
    else
        cs->addFormFile(kStrData, payload.fileName, mimeType);

    cs->addFormField(kStrWidth, toString(flipbook.width));
    cs->addFormField(kStrHeight, toString(flipbook.height));
    cs->addFormField(kStrNumberOfFrames, toString(flipbook.numberOfFrames));
    cs->addFormField(kStrContentType, mimeType);

    std::string url = getVideosUrl(accountId, instrumentId);

    CURLcode res = cs->httpPostForm(url);

    if (res != CURLE_OK)
    {
        LOG(ERROR) << "POST " << url << " failed: " << curl_easy_strerror(res);
        return STATUS_ERROR;
    }

    if (cs->getResponseCode() != 201)
    {
        LOG(ERROR) << "uploadFlipbook() failed, response code: "
               << cs->getResponseCode() << ", error message: "
               << cs->getErrorMessage();
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

status_t Client::Impl::uploadEvent(id_t accountId, id_t instrumentId,
                                   const timestamp_t& timestamp, const Events& data)
{
    CurlSessionPtr session = CurlSession::create(token_);

    if (!session)
        return STATUS_ERROR;

    CurlSession* cs = session.get();

//    -F "key=EVENT"
    cs->addFormField(kStrKey, kStrEVENT);

    //    -F "timestamp=2016-08-17T00:00:00"
    cs->addFormField(kStrTimestamp, toIsoTimeString(timestamp));

    //    -F "data=<json_as_std::string>;type=application/json"
    std::string json = toJsonString(data);
    cs->addFormField(kStrData, json, "application/json");

    std::string url = getTimeSeriesUrl(accountId, instrumentId);

    CURLcode res = cs->httpPostForm(url);

    if (res != CURLE_OK)
    {
        LOG(ERROR) << "POST " << url << " failed: " << curl_easy_strerror(res);
        return STATUS_ERROR;
    }

    if (cs->getResponseCode() != 201)
    {
        LOG(ERROR) << "uploadEvent() failed, response code: "
               << cs->getResponseCode() << ", error message: "
               << cs->getErrorMessage();
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

status_t Client::Impl::uploadObjectStream(id_t accountId, id_t instrumentId,
                                          const ObjectStream& stream, const Payload& payload)
{
    CurlSessionPtr session = CurlSession::create(token_);

    if (!session)
        return STATUS_ERROR;

    CurlSession* cs = session.get();

    cs->addFormField(kStrKey, kStrOBJECT_STREAM);

    std::string json = toJsonString(stream);
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
        return STATUS_ERROR;
    }

    if (cs->getResponseCode() != 201)
    {
        LOG(ERROR) << "uploadObjectStream() failed, response code: "
               << cs->getResponseCode() << ", error message: "
               << cs->getErrorMessage();
        return STATUS_ERROR;
    }

    return STATUS_OK;
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

}
}
