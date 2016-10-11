#include "api/client.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include <cpr/cpr.h>
#include <json.hpp>

namespace prism {
namespace connect {
namespace api {

class Client::Impl {
  public:
    Impl(const std::string& api_root, const std::string& api_token);

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
    auto response_json = nlohmann::json::parse(response.text);

    version_ = response_json["version"].get<std::string>();
    accounts_url_ = response_json["accounts_url"].get<std::string>();
}

Client::Client(const std::string& api_root, const std::string& api_token)
        : pimpl_{new Impl{api_root, api_token}} {}

Client::~Client() {}

} // namespace api
} // namespace connect
} // namespace prism
