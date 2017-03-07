/*
 * Copyright (C) 2016-2017 Prism Skylabs
 */
#include "domain-types.h"

namespace prism
{
namespace connect
{

std::ostream&operator<<(std::ostream& os, const Status& status)
{
    os << std::hex << status.status_ << " (" << (status.isError() ? 'E' : 'S') << std::dec;

    if (status.getFacility() != Status::FACILITY_NONE)
        os << ", facility: " << status.getFacility();

    os << ", code: " << status.getCode() << ") ";

    return os;
}

}
}
