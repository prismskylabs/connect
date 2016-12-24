#include "client.h"
#include "curl-session.h"
#include "easylogging++.h"
#include "rapidjson/document.h"

namespace prism {
namespace connect {

const char* kStrId     = "id";
const char* kStrName   = "name";
const char* kStrUrl    = "url";
const char* kStrInstrumentsUrl = "instruments_url";
const char* kStrVersion = "version";
const char* kStrAccountsUrl = "accounts_url";

class Client::Impl {
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

private:
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
}

status_t Client::init() { return pImpl_->init(); }

status_t Client::queryApiState(string& accountsUrl, string& apiVersion) {
    return STATUS_ERROR;
}

status_t Client::queryAccountsList(AccountsList& accounts) {
    return pImpl_->queryAccountsList(accounts);
}

status_t Client::queryAccount(id_t accountId, Account &account) {
    return pImpl_->queryAccount(accountId, account);
}

status_t Client::queryInstrumentsList(id_t accountId, InstrumentsList &instruments) {
    return pImpl_->queryInstrumentsList(accountId, instruments);
}

bool hasStringMember(const rapidjson::Value& value, const char* name) {
    return value.HasMember(name)  &&  value[name].IsString();
}

bool hasIntMember(const rapidjson::Value& value, const char* name) {
    return value.HasMember(name)  &&  value[name].IsInt();
}

status_t Client::Impl::init()
{
    CurlSessionPtr session = CurlSession::create(token_);

    if (!session)
        return STATUS_ERROR;

    CURLcode res = session->httpGet(apiRoot_);

    if (res != CURLE_OK) {
        LERROR << "GET " << apiRoot_ << " failed: " << curl_easy_strerror(res);
        return STATUS_ERROR;
    }

    const string& responseBody = session->getResponseBodyAsString();

    rapidjson::Document document;
    document.Parse(responseBody.c_str());

    if (hasStringMember(document, kStrAccountsUrl)
            &&  hasStringMember(document, kStrUrl)
            &&  hasStringMember(document, kStrVersion)) {
        accountsUrl_ = document[kStrAccountsUrl].GetString();
    } else {
        LERROR << "Response JSON must contain three string members: "
               << kStrAccountsUrl << ", " << kStrUrl << " and " << kStrVersion;
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

status_t parseAccount(const rapidjson::Value& itemJson, Account& account) {
    if (!hasIntMember(itemJson, kStrId)) {
        LERROR << "Account JSON must contain integer member " << kStrId;
        return STATUS_ERROR;
    }

    if (!hasStringMember(itemJson, kStrName)
            ||  !hasStringMember(itemJson, kStrUrl)
            ||  !hasStringMember(itemJson, kStrInstrumentsUrl)) {
        LERROR << "Account must have string members " << kStrName
               << ", " << kStrUrl << " and " << kStrInstrumentsUrl;
        return STATUS_ERROR;
    }

    account.id = itemJson[kStrId].GetInt();
    account.name = itemJson[kStrName].GetString();
    account.url = itemJson[kStrUrl].GetString();
    account.instrumentsUrl = itemJson[kStrInstrumentsUrl].GetString();

    return STATUS_OK;
}

status_t Client::Impl::queryAccountsList(AccountsList &accounts)
{
    CurlSessionPtr session = CurlSession::create(token_);

    if (!session)
        return STATUS_ERROR;

    CURLcode res = session->httpGet(accountsUrl_);

    if (res != CURLE_OK) {
        LERROR << "GET " << accountsUrl_ << " failed: " << curl_easy_strerror(res);
        return STATUS_ERROR;
    }

    const string& responseBody = session->getResponseBodyAsString();

    LINFO << responseBody;

    rapidjson::Document document;
    document.Parse(responseBody.c_str());

    if (!document.IsArray()) {
        LERROR << "Accounts list must be JSON array";
        return STATUS_ERROR;
    }

    for (rapidjson::SizeType i = 0; i < document.Size(); ++i) {
        accounts.push_back(Account());
        status_t status = parseAccount(document[i], accounts.back());

        if (status != STATUS_OK)
            return status;
    }

    return STATUS_OK;
}

string toString(id_t id) {
    const size_t bufSize = 16;
    char buf[bufSize];
    snprintf(buf, bufSize, "%d", id);
    return string(buf);
}

status_t Client::Impl::queryAccount(id_t accountId, Account &account) {
    CurlSessionPtr session = CurlSession::create(token_);

    if (!session)
        return STATUS_ERROR;

    string accountUrl = accountsUrl_ + toString(accountId);

    CURLcode res = session->httpGet(accountUrl);

    if (res != CURLE_OK) {
        LERROR << "GET " << accountUrl << " failed: " << curl_easy_strerror(res);
        return STATUS_ERROR;
    }

    const string& responseBody = session->getResponseBodyAsString();

    LINFO << responseBody;

    rapidjson::Document document;

    if (document.Parse(responseBody.c_str()).HasParseError()) {
        LERROR << "Error parsing account JSON";
        return STATUS_ERROR;
    }

    return parseAccount(document, account);
}

status_t parseInstrument(const rapidjson::Value& itemJson, Instrument& instrument) {
//    if (!hasIntMember(itemJson, kStrId)) {
//        LERROR << "Account JSON must contain integer member " << kStrId;
//        return STATUS_ERROR;
//    }

//    if (!hasStringMember(itemJson, kStrName)
//            ||  !hasStringMember(itemJson, kStrUrl)
//            ||  !hasStringMember(itemJson, kStrInstrumentsUrl)) {
//        LERROR << "Account must have string members " << kStrName
//               << ", " << kStrUrl << " and " << kStrInstrumentsUrl;
//        return STATUS_ERROR;
//    }

//    account.id = itemJson[kStrId].GetInt();
//    account.name = itemJson[kStrName].GetString();
//    account.url = itemJson[kStrUrl].GetString();
//    account.instrumentsUrl = itemJson[kStrInstrumentsUrl].GetString();

    return STATUS_OK;
}

status_t Client::Impl::queryInstrumentsList(id_t accountId, InstrumentsList &instruments)
{
    CurlSessionPtr session = CurlSession::create(token_);

    if (!session)
        return STATUS_ERROR;

    string url = accountsUrl_ + toString(accountId) + "/instruments/";

    CURLcode res = session->httpGet(url);

    if (res != CURLE_OK) {
        LERROR << "GET " << url << " failed: " << curl_easy_strerror(res);
        return STATUS_ERROR;
    }

    const string& responseBody = session->getResponseBodyAsString();

    LINFO << responseBody;

    rapidjson::Document document;

    if (document.Parse(responseBody.c_str()).HasParseError()) {
        LERROR << "Error parsing instruments list JSON";
        return STATUS_ERROR;
    }

    if (!document.IsArray()) {
        LERROR << "Instruments list must be JSON array";
        return STATUS_ERROR;
    }

    for (rapidjson::SizeType i = 0; i < document.Size(); ++i) {
        instruments.push_back(Instrument());
        status_t status = parseInstrument(document[i], instruments.back());

        if (status != STATUS_OK)
            return status;
    }

    return STATUS_OK;
}

}
}
