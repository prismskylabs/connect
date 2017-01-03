#ifndef CONNECT_SDK_COMMON_TYPES_H
#define CONNECT_SDK_COMMON_TYPES_H

#include <string>
#include <vector>
#include <map>
#include <boost/move/unique_ptr.hpp>
#include <boost/chrono/system_clocks.hpp>

namespace prism {
namespace connect {

using std::string;
using std::vector;
using std::map;

using boost::movelib::unique_ptr;
namespace chrono = boost::chrono;

typedef chrono::system_clock::time_point timestamp_t;

}
}

#endif // CONNECT_SDK_COMMON_TYPES_H
