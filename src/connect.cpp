#include "connect.h"

namespace prism {
namespace connect {

class Client::Impl {
public:
    Impl(const string& apiRoot, const string& token)
        : apiRoot_(apiRoot)
        , token_(token)
    {
    }

private:
    string apiRoot_;
    string token_;
};

Client::Client(const string& apiRoot, const string& token)
    : pImpl_(new Impl(apiRoot, token))
{
}

Client::status_t Client::queryAccountsList(AccountsList& accounts)
{

}

}
}

