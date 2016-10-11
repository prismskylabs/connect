#ifndef PRISM_CONNECT_API_Account_H_
#define PRISM_CONNECT_API_Account_H_

#include <cstdint>
#include <string>

#include <json.hpp>

namespace prism {
namespace connect {
namespace api {

class Account {
  public:
    Account();
    Account(const nlohmann::json& account_json);
    Account(const std::uint32_t id, const std::string& name, const std::string& url,
            const std::string& instruments_url);

    explicit operator bool() const;

    std::uint32_t id_;
    std::string name_;
    std::string url_;
    std::string instruments_url_;
};

} // namespace api
} // namespace connect
} // namespace prism

#endif // PRISM_CONNECT_API_Account_H_
