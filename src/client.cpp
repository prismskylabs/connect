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
    Impl(const string& apiRoot, const string& token)
        : apiRoot_(apiRoot)
        , token_(token)
    {
    }

    status_t init();

    status_t queryAccountsList(AccountsList& accounts);
    status_t queryAccount(id_t accountId, Account &account);
    status_t queryInstrumentsList(id_t accountId, InstrumentsList& instruments);
    status_t registerInstrument(id_t accountId, const Instrument& instrument);

    status_t uploadFlipbook(id_t accountId, id_t instrumentId,
                            const Flipbook& flipbook);

    status_t uploadEvent(id_t accountId, id_t instrumentId,
                         const timestamp_t& timestamp, const EventData& data);

    status_t uploadObjectStream(id_t accountId, id_t instrumentId,
                                const ObjectStream& stream, const string& imageFile);

private:
    string getInstrumentsUrl(id_t accountId) const;
    string getAccountUrl(id_t accountId) const;
    string getInstrumentUrl(id_t accountId, id_t instrumentId) const;
    string getVideosUrl(id_t accountId, id_t instrumentId) const;
    string getImagesUrl(id_t accountId, id_t instrumentId) const;
    string getTimeSeriesUrl(id_t accountId, id_t instrumentId) const;

    string apiRoot_;
    string token_;

    string accountsUrl_;
};

Client::Client(const string& apiRoot, const string& token)
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

status_t Client::queryApiState(string& accountsUrl, string& apiVersion)
{
    return STATUS_ERROR;
}

status_t Client::queryAccountsList(AccountsList& accounts)
{
    return pImpl_->queryAccountsList(accounts);
}

status_t Client::queryAccount(id_t accountId, Account &account)
{
    return pImpl_->queryAccount(accountId, account);
}

status_t Client::queryInstrumentsList(id_t accountId, InstrumentsList &instruments)
{
    return pImpl_->queryInstrumentsList(accountId, instruments);
}

status_t Client::registerInstrument(id_t accountId, const Instrument& instrument)
{
    return pImpl_->registerInstrument(accountId, instrument);
}

status_t Client::uploadObjectStream(id_t accountId, id_t instrumentId,
                                    const ObjectStream& stream, const string& imageFile)
{
    return pImpl_->uploadObjectStream(accountId, instrumentId, stream, imageFile);
}

status_t Client::uploadFlipbook(id_t accountId, id_t instrumentId,
                                const Flipbook& flipbook)
{
    return pImpl_->uploadFlipbook(accountId, instrumentId, flipbook);
}

status_t Client::uploadEvent(id_t accountId, id_t instrumentId,
                             const timestamp_t& timestamp, const EventData& data)
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
        LERROR << "GET " << apiRoot_ << " failed: " << curl_easy_strerror(res);
        return STATUS_ERROR;
    }

    const string& responseBody = session->getResponseBodyAsString();

    rapidjson::Document document;

    if (document.Parse(responseBody.c_str()).HasParseError())
    {
        LERROR << "Error parsing response";
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
        LERROR << "Response JSON must contain three string members: "
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
        LERROR << "Account JSON must contain integer member " << kStrId;
        return STATUS_ERROR;
    }

    if (!hasStringMember(itemJson, kStrName)
        ||  !hasStringMember(itemJson, kStrUrl)
        ||  !hasStringMember(itemJson, kStrInstrumentsUrl))

    {
        LERROR << "Account must have string members " << kStrName
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

status_t Client::Impl::queryAccountsList(AccountsList& accounts)
{
    CurlSessionPtr session = CurlSession::create(token_);

    if (!session)
        return STATUS_ERROR;

    CURLcode res = session->httpGet(accountsUrl_);

    if (res != CURLE_OK)
    {
        LERROR << "GET " << accountsUrl_ << " failed: " << curl_easy_strerror(res);
        return STATUS_ERROR;
    }

    const string& responseBody = session->getResponseBodyAsString();

    LINFO << responseBody;

    rapidjson::Document document;

    if (document.Parse(responseBody.c_str()).HasParseError())
    {
        LERROR << "Error parsing accounts list JSON";
        return STATUS_ERROR;
    }

    if (!document.IsArray())
    {
        LERROR << "Accounts list must be JSON array";
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

    string url = getAccountUrl(accountId);

    CURLcode res = session->httpGet(url);

    if (res != CURLE_OK)
    {
        LERROR << "GET " << url << " failed: " << curl_easy_strerror(res);
        return STATUS_ERROR;
    }

    const string& responseBody = session->getResponseBodyAsString();

    LINFO << responseBody;

    rapidjson::Document document;

    if (document.Parse(responseBody.c_str()).HasParseError())
    {
        LERROR << "Error parsing account JSON";
        return STATUS_ERROR;
    }

    return parseAccount(document, account);
}

status_t parseInstrument(const rapidjson::Value& itemJson, Instrument& instrument)
{
    if (!hasStringMember(itemJson, kStrName)
        ||  !hasStringMember(itemJson, kStrInstrumentType))
    {
        LERROR << "Instrument must have string members " << kStrName
               << " and " << kStrInstrumentType;
        return STATUS_ERROR;
    }

    // TODO reset instrument members, not set here, may be with instrument.clear()
    instrument.name = itemJson[kStrName].GetString();
    instrument.type = itemJson[kStrInstrumentType].GetString();

    return STATUS_OK;
}

status_t Client::Impl::queryInstrumentsList(id_t accountId, InstrumentsList& instruments)
{
    CurlSessionPtr session = CurlSession::create(token_);

    if (!session)
        return STATUS_ERROR;

    string url = getInstrumentsUrl(accountId);

    CURLcode res = session->httpGet(url);

    if (res != CURLE_OK)
    {
        LERROR << "GET " << url << " failed: " << curl_easy_strerror(res);
        return STATUS_ERROR;
    }

    const string& responseBody = session->getResponseBodyAsString();

    LINFO << responseBody;

    rapidjson::Document document;

    if (document.Parse(responseBody.c_str()).HasParseError())
    {
        LERROR << "Error parsing instruments list JSON";
        return STATUS_ERROR;
    }

    if (!document.IsArray())
    {
        LERROR << "Instruments list must be JSON array";
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

    string url = getInstrumentsUrl(accountId);

    session->addHeader("Content-Type: application/json");

    CURLcode res = session->httpPost(url, toJsonString(instrument));

    if (res != CURLE_OK)
    {
        LERROR << "POST " << url << " failed: " << curl_easy_strerror(res);
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

status_t Client::Impl::uploadFlipbook(id_t accountId, id_t instrumentId, const Flipbook& flipbook)
{
    CurlSessionPtr session = CurlSession::create(token_);

    if (!session)
        return STATUS_ERROR;

    CurlSession* cs = session.get();

    //    -F "key=FLIPBOOK"
    cs->addFormField(kStrKey, kStrFLIPBOOK);

    //    -F "start_timestamp=2016-08-17T00:00:00"
    cs->addFormField(kStrStartTimestamp, toIsoTimeString(flipbook.startTimestamp));
//    cs->addFormField(kStrStartTimestamp, "2016-08-17T00:00:00.12");

    //    -F "stop_timestamp=2016-08-17T00:00:00"
    cs->addFormField(kStrStopTimestamp, toIsoTimeString(flipbook.stopTimestamp));
//    cs->addFormField(kStrStopTimestamp, "2016-08-17T00:00:00.34");

    string mimeType = mimeTypeFromFilePath(flipbook.videoFile);

    //    -F "data=@/path/to/flipbook.mp4;type=video/mp4"
    cs->addFormFile(kStrData, flipbook.videoFile, mimeType);

    //    -F "width=1280"
    cs->addFormField(kStrWidth, toString(flipbook.width));

    //    -F "height=720"
    cs->addFormField(kStrHeight, toString(flipbook.height));

    //    -F "number_of_frames=60"
    cs->addFormField(kStrNumberOfFrames, toString(flipbook.numberOfFrames));

    //    -F "content_type=video/mp4"
    cs->addFormField(kStrContentType, mimeType);

    string url = getVideosUrl(accountId, instrumentId);

    CURLcode res = cs->httpPostForm(url);

    if (res != CURLE_OK)
    {
        LERROR << "POST " << url << " failed: " << curl_easy_strerror(res);
        return STATUS_ERROR;
    }

    if (cs->getResponseCode() != 201)
    {
        LERROR << "uploadFlipbook() failed, response code: "
               << cs->getResponseCode() << ", error message: "
               << cs->getErrorMessage();
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

status_t Client::Impl::uploadEvent(id_t accountId, id_t instrumentId,
                                   const timestamp_t& timestamp, const EventData& data)
{
    CurlSessionPtr session = CurlSession::create(token_);

    if (!session)
        return STATUS_ERROR;

    CurlSession* cs = session.get();

//    -F "key=EVENT"
    cs->addFormField(kStrKey, kStrEVENT);

    //    -F "timestamp=2016-08-17T00:00:00"
    cs->addFormField(kStrTimestamp, toIsoTimeString(timestamp));

    //    -F "data=<json_as_string>;type=application/json"
    string json = toJsonString(data);
    cs->addFormField(kStrData, json, "application/json");

    string url = getTimeSeriesUrl(accountId, instrumentId);

    CURLcode res = cs->httpPostForm(url);

    if (res != CURLE_OK)
    {
        LERROR << "POST " << url << " failed: " << curl_easy_strerror(res);
        return STATUS_ERROR;
    }

    if (cs->getResponseCode() != 201)
    {
        LERROR << "uploadEvent() failed, response code: "
               << cs->getResponseCode() << ", error message: "
               << cs->getErrorMessage();
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

status_t Client::Impl::uploadObjectStream(id_t accountId, id_t instrumentId,
                                          const ObjectStream& stream, const string& imageFile)
{
    CurlSessionPtr session = CurlSession::create(token_);

    if (!session)
        return STATUS_ERROR;

    CurlSession* cs = session.get();

//    -F "key=OBJECT_STREAM"
    cs->addFormField(kStrKey, kStrOBJECT_STREAM);

//    -F "meta=<meta_information_json>"
    string json = toJsonString(stream);
    cs->addFormField(kStrMeta, json, "application/json");

//    -F "data=@/path/to/image.png;type=image/png"
    cs->addFormFile(kStrData, imageFile, mimeTypeFromFilePath(imageFile));

    string url = getImagesUrl(accountId, instrumentId);

    CURLcode res = cs->httpPostForm(url);

    if (res != CURLE_OK)
    {
        LERROR << "POST " << url << " failed: " << curl_easy_strerror(res);
        return STATUS_ERROR;
    }

    if (cs->getResponseCode() != 201)
    {
        LERROR << "uploadObjectStream() failed, response code: "
               << cs->getResponseCode() << ", error message: "
               << cs->getErrorMessage();
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

string Client::Impl::getInstrumentsUrl(id_t accountId) const
{
    return accountsUrl_ + toString(accountId) + "/instruments/";
}

string Client::Impl::getAccountUrl(id_t accountId) const
{
    return accountsUrl_ + toString(accountId) + '/';
}

string Client::Impl::getInstrumentUrl(id_t accountId, id_t instrumentId) const
{
    return getInstrumentsUrl(accountId) + toString(instrumentId) + '/';
}

string Client::Impl::getVideosUrl(id_t accountId, id_t instrumentId) const
{
    return getInstrumentUrl(accountId, instrumentId) + "data/videos/";
}

string Client::Impl::getImagesUrl(id_t accountId, id_t instrumentId) const
{
    return getInstrumentUrl(accountId, instrumentId) + "data/images/";
}

string Client::Impl::getTimeSeriesUrl(id_t accountId, id_t instrumentId) const
{
    return getInstrumentUrl(accountId, instrumentId) + "data/time-series/";
}

}
}
