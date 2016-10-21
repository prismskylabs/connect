#ifndef PRISM_CONNECT_PROCESSORS_Track_H_
#define PRISM_CONNECT_PROCESSORS_Track_H_

#include <json.hpp>

namespace prism {
namespace connect {
namespace processors {

class Track {
  public:
    Track(int id, float x, float y, float w, float h);

    void SetCentroid(float x, float y);
    void SetFootPoint(float x, float y);
    void SetVelocity(float x, float y);
    void SetAge(int age_ms);
    void SetArea(float area);
    void SetParents(const std::vector<int>& parent_ids);
    void SetEqualId(int equal_id);

    nlohmann::json ToJson() const;

  private:
    nlohmann::json data_;
};

} // namespace processors
} // namespace connect
} // namespace prism

#endif // PRISM_CONNECT_PROCESSORS_Track_H_
