#ifndef CONNECT_SDK_UTIL_H
#define CONNECT_SDK_UTIL_H

#include "common-types.h"

namespace prism
{
namespace connect
{
    class Metadata;

    string toJsonString(const Metadata& metadata);
}
}

#endif // CONNECT_SDK_UTIL_H
