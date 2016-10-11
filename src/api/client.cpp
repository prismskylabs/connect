#include "api/client.h"

#include <cstdint>
#include <vector>
#include <string>

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
};

Client::Impl::Impl(const std::string& api_root, const std::string& api_token)
        : api_root_{api_root}, api_token_{api_token} {
    std::cout << "Created client with root: " << api_root_ << " and token: " << api_token_
              << std::endl;
}

Client::Client(const std::string& api_root, const std::string& api_token)
        : pimpl_{new Impl{api_root, api_token}} {}

Client::~Client() {}

} // namespace api
} // namespace connect
} // namespace prism
