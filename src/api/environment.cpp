#include "api/environment.h"

#include <cstdlib>
#include <string>
#include <stdexcept>

namespace prism {
namespace connect {
namespace api {
namespace environment {

std::string ApiRoot() {
    static std::string api_root;
    if (api_root.empty()) {
        if (const char* api_root_raw = std::getenv("API_ROOT")) {
            api_root = api_root_raw;
        } else {
            throw std::runtime_error("No API_ROOT environment variable set");
        }
    }

    return api_root;
}

std::string ApiToken() {
    static std::string api_token;
    if (api_token.empty()) {
        if (const char* api_token_raw = std::getenv("API_TOKEN")) {
            api_token = api_token_raw;
        } else {
            throw std::runtime_error("No API_TOKEN environment variable set");
        }
    }

    return api_token;
}

} // namespace environment
} // namespace api
} // namespace connect
} // namespace prism
