#include "api/account.h"

#include <cstdint>
#include <string>

#include <json.hpp>

namespace prism {
namespace connect {
namespace api {

Account::Account() : id_{0} {}

Account::Account(const nlohmann::json& account_json)
        : Account{account_json["id"].get<std::uint32_t>(), account_json["name"].get<std::string>(),
                  account_json["url"].get<std::string>(),
                  account_json["instruments_url"].get<std::string>()} {}

Account::Account(const std::uint32_t id, const std::string& name, const std::string& url,
                 const std::string& instruments_url)
        : id_{id}, name_{name}, url_{url}, instruments_url_{instruments_url} {}

Account::operator bool() const {
    return id_ > 0;
}

} // namespace api
} // namespace connect
} // namespace prism
