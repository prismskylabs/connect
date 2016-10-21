#include "processors/track-collector.h"

#include <chrono>
#include <vector>

#include <json.hpp>

#include "processors/track.h"
#include "util/time.h"

namespace prism {
namespace connect {
namespace processors {

void TrackCollector::AddTracks(const std::chrono::system_clock::time_point& timestamp,
               const std::vector<Track>& tracks) {
    nlohmann::json track_set;
    track_set["timestamp"] = util::IsoTime(timestamp);

    nlohmann::json tracks_to_add;
    for (const auto& track : tracks) {
        tracks_to_add.push_back(track.ToJson());
    }
    track_set["tracks"] = tracks_to_add;

    tracks_.push_back(track_set);
}

nlohmann::json TrackCollector::GetTracksJson() const {
    return tracks_;
}

void TrackCollector::ClearTracks() {
    tracks_.clear();
}

} // namespace processors
} // namespace connect
} // namespace prism
