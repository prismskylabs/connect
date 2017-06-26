/*
 * Copyright (C) 2016-2017 Prism Skylabs
 */
#include "domain-types.h"
#include <sstream>

namespace prism
{
namespace connect
{

std::ostream&operator<<(std::ostream& os, const Status& status)
{
    os << status.toString();
    return os;
}

std::string Status::toString() const
{
    std::stringstream ss;

    ss << std::hex << status_ << " (" << (isError() ? 'E' : 'S') << std::dec;

    if (getFacility() != Status::FACILITY_NONE)
        ss << ", facility: " << getFacility();

    ss << ", code: " << getCode() << ") ";

    return ss.str();
}

}
}
