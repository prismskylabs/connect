#include "processors/track-collector.h"

#include <json.hpp>

#include "processors/track.h"
#include "util/time.h"

namespace prism {
namespace connect {
namespace processors {

void TrackCollector::AddTrack(const Track& track) {
    tracks_.push_back(track.ToJson());
}

int TrackCollector::GetNumberOfTracks() const {
    return tracks_.size();
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
