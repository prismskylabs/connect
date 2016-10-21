#include "processors/track.h"

#include <json.hpp>

namespace prism {
namespace connect {
namespace processors {

Track::Track(int id, float x, float y, float w, float h) {
    data_["id"] = id;
    data_["bb"] = {x, y, w, h};
}

void Track::SetCentroid(float x, float y) {
    data_["centroid"] = {x, y};
}

void Track::SetFootPoint(float x, float y) {
    data_["foot_point"] = {x, y};
}

void Track::SetVelocity(float x, float y) {
    data_["velocity"] = {x, y};
}

void Track::SetAge(int age_ms) {
    data_["age"] = age_ms;
}

void Track::SetArea(float area) {
    data_["area"] = area;
}

void Track::SetParents(const std::vector<int>& parent_ids) {
    data_["parents"] = parent_ids;
}

void Track::SetEqualId(int equal_id) {
    data_["equal_id"] = equal_id;
}

nlohmann::json Track::ToJson() const {
    return data_;
}

} // namespace processors
} // namespace connect
} // namespace prism
