#ifndef PRISM_CONNECT_PROCESSORS_TrackCollector_H_
#define PRISM_CONNECT_PROCESSORS_TrackCollector_H_

#include <json.hpp>

#include "processors/track.h"

namespace prism {
namespace connect {
namespace processors {

class TrackCollector {
  public:
    void AddTrack(const Track& track);
    int GetNumberOfTracks() const;
    nlohmann::json GetTracksJson() const;
    void ClearTracks();

  private:
    nlohmann::json tracks_;
};

} // namespace processors
} // namespace connect
} // namespace prism

#endif // PRISM_CONNECT_PROCESSORS_TrackCollector_H_
