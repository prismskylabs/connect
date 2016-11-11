#ifndef PRISM_CONNECT_API_Response_H_
#define PRISM_CONNECT_API_Response_H_

#include <cstdint>
#include <string>

namespace prism {
namespace connect {
namespace api {

typedef struct {
    std::int32_t status_code;
    std::string text;
} Response;

} // namespace api
} // namespace connect
} // namespace prism

#endif // PRISM_CONNECT_API_Response_H_
