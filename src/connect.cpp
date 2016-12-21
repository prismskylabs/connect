#include "connect.h"
#include "curl/curl.h"
#include "easylogging++.h"
#include "rapidjson/document.h"

namespace prism {
namespace connect {

size_t writeFunction(void* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append((char*) ptr, size * nmemb);
    return size * nmemb;
}

class Client::Impl {
public:
    Impl(const string& apiRoot, const string& token)
        : apiRoot_(apiRoot)
        , token_(token)
    {
    }

    ~Impl() {
        if (curl_) {
            curl_easy_cleanup(curl_);
            curl_ = nullptr;
        }
    }

    Client::status_t init() {
        curl_ = curl_easy_init();

        if (!curl_) {
            LERROR << "curl_easy_init() failed";
            return STATUS_ERROR;
        }

        curl_easy_setopt(curl_, CURLOPT_URL, apiRoot_.c_str());
        curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1);
//        curl_easy_setopt(curl_, CURLOPT_HEADER, 1);
        curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl_, CURLOPT_MAXREDIRS, 10);

        struct curl_slist* chunk = NULL;
        chunk = curl_slist_append(chunk, string("Authorization: Token ").append(token_).c_str());

        CURLcode res = curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, chunk);

        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &responseBody_);
        curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, writeFunction);
        curl_easy_setopt(curl_, CURLOPT_HEADERDATA, &responseHeaders_);

        responseBody_.clear();
        responseHeaders_.clear();

        res = curl_easy_perform(curl_);

        curl_slist_free_all(chunk);

        if (res != CURLE_OK) {
              LERROR << "curl_easy_perform() failed: " << curl_easy_strerror(res);
              return STATUS_ERROR;
        }

        rapidjson::Document document;
        document.Parse(responseBody_.c_str());

        if (!document.HasMember("accounts_url") || !document.HasMember("url") || !document.HasMember("version")) {
            LERROR << "Response must have all of \"accounts_url\", \"url\" and \"version\" members";
            return STATUS_ERROR;
        }

        if (!document["accounts_url"].IsString() || !document["url"].IsString() || !document["version"].IsString()) {
            LERROR << "All response members must be strings";
            return STATUS_ERROR;
        }

        accountsUrl_ = document["accounts_url"].GetString();

        return STATUS_OK;
    }

private:
    string apiRoot_;
    string token_;

    string accountsUrl_;

    CURL* curl_;
    string responseBody_;
    string responseHeaders_;
    char error[CURL_ERROR_SIZE];
};

Client::Client(const string& apiRoot, const string& token)
    : pImpl_(new Impl(apiRoot, token))
{
}

Client::~Client()
{
}

Client::status_t Client::init() { return pImpl_->init(); }

Client::status_t Client::queryApiState(string& accountsUrl, string& apiVersion)
{

}

Client::status_t Client::queryAccountsList(AccountsList& accounts)
{

}

class Metadata::Impl {

};

Metadata::~Metadata()
{

}

}
}

