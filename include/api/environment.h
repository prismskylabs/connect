#ifndef PRISM_CONNECT_API_Environment_H_
#define PRISM_CONNECT_API_Environment_H_

#include <iostream>

namespace prism {
namespace connect {
namespace api {
namespace environment {

// API_ROOT and API_TOKEN are preprocessor definitions taken from the project build file

static const std::string ApiRoot = API_ROOT;
static const std::string ApiToken = API_TOKEN;

} // naespace environment
} // namespace api
} // namespace connect
} // namespace prism

#endif // PRISM_CONNECT_API_Environment_H_
