#include "api/client.h"

#include <cstdint>
#include <vector>
#include <string>
#include <stdexcept>

#include <cpr/cpr.h>
#include <json.hpp>

#include "api/account.h"

namespace prism {
namespace connect {
namespace api {

class Client::Impl {
  public:
    Impl(const std::string& api_root, const std::string& api_token);
    std::vector<Account> QueryAccounts();
    Account QueryAccount(const std::uint32_t id);

  private:
    std::string api_root_;
    std::string api_token_;
    cpr::Session session_;

    std::string version_;
    std::string accounts_url_;
};

Client::Impl::Impl(const std::string& api_root, const std::string& api_token)
        : api_root_{api_root}, api_token_{api_token} {
    session_.SetUrl(api_root);
    session_.SetHeader({{"Authorization", std::string{"Token "} + api_token_}});

    auto response = session_.Get();
    if (!response.error && response.status_code == 200) {
        auto response_json = nlohmann::json::parse(response.text);
        version_ = response_json["version"].get<std::string>();
        accounts_url_ = response_json["accounts_url"].get<std::string>();
    }
}

std::vector<Account> Client::Impl::QueryAccounts() {
    std::vector<Account> accounts;

    session_.SetUrl(accounts_url_);
    auto response = session_.Get();

    if (!response.error && response.status_code == 200) {
        auto response_json = nlohmann::json::parse(response.text);

        for (const auto& account_json : response_json) {
            accounts.emplace_back(account_json);
        }
    }

    return accounts;
}

Account Client::Impl::QueryAccount(const std::uint32_t id) {
    session_.SetUrl(accounts_url_ + std::to_string(id) + "/");
    auto response = session_.Get();

    if (!response.error && response.status_code == 200) {
        auto account_json = nlohmann::json::parse(response.text);
        return Account{account_json};
    }

    return Account{0, "", "", ""};
}

Client::Client(const std::string& api_root, const std::string& api_token)
        : pimpl_{new Impl{api_root, api_token}} {}

Client::~Client() {}

std::vector<Account> Client::QueryAccounts() {
    return pimpl_->QueryAccounts();
}

Account Client::QueryAccount(const std::uint32_t id) {
    return pimpl_->QueryAccount(id);
}

} // namespace api
} // namespace connect
} // namespace prism
