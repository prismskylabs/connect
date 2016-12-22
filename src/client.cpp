#include "client.h"
#include "curl-session.h"
#include "easylogging++.h"
#include "rapidjson/document.h"

namespace prism {
namespace connect {

class Client::Impl {
public:
    Impl(const string& apiRoot, const string& token)
        : apiRoot_(apiRoot)
        , token_(token)
    {
    }

    status_t init();

    status_t queryAccountsList(AccountsList& accounts);

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

status_t Client::queryApiState(string& accountsUrl, string& apiVersion)
{
    return STATUS_ERROR;
}

status_t Client::queryAccountsList(AccountsList& accounts)
{
    return pImpl_->queryAccountsList(accounts);
}

bool hasStringMember(const rapidjson::Document& document, const char* name) {
    return document.HasMember(name)  &&  document[name].IsString();
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

    if (hasStringMember(document, "accounts_url")
            &&  hasStringMember(document, "url")
            &&  hasStringMember(document, "version")) {
        accountsUrl_ = document["accounts_url"].GetString();
    } else {
        LERROR << "Response must have three string members: \"accounts_url\", \"url\" and \"version\"";
        return STATUS_ERROR;
    }

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

//    if (hasStringMember(document, "accounts_url")
//            &&  hasStringMember(document, "url")
//            &&  hasStringMember(document, "version")) {
//        accountsUrl_ = document["accounts_url"].GetString();
//    } else {
//        LERROR << "Response must have three string members: \"accounts_url\", \"url\" and \"version\"";
//        return STATUS_ERROR;
//    }

    return STATUS_OK;
}

}
}
