#include "processors/track.h"

#include <chrono>

#include <json.hpp>

#include "util/time.h"

namespace prism {
namespace connect {
namespace processors {

Track::Track(int id, const std::chrono::system_clock::time_point& start_time)
        : last_time_{start_time} {
    data_["object_id"] = id;
    data_["timestamp"] = util::IsoTime(last_time_);
    data_["points"] = nlohmann::json::array();
}

void Track::AddPoint(const int x, const int y,
                     const std::chrono::system_clock::time_point& timestamp) {
    data_["points"].push_back(
            {x, y, std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - last_time_)
                           .count()});
    last_time_ = timestamp;
}

void Track::AddPoint(const int x, const int y, const int time_difference) {
    data_["points"].push_back({x, y, time_difference});
}

nlohmann::json Track::ToJson() const {
    return data_;
}

} // namespace processors
} // namespace connect
} // namespace prism
