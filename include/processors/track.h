#ifndef PRISM_CONNECT_PROCESSORS_Track_H_
#define PRISM_CONNECT_PROCESSORS_Track_H_

#include <chrono>

#include <json.hpp>

namespace prism {
namespace connect {
namespace processors {

typedef struct {
    float x;
    float y;
    int td_milliseconds;
} TrackPoint;

class Track {
  public:
    Track(int id, const std::chrono::system_clock::time_point& start_time);

    void AddPoint(const float x, const float y,
                  const std::chrono::system_clock::time_point& timestamp);

    nlohmann::json ToJson() const;

  private:
    nlohmann::json data_;
    std::chrono::system_clock::time_point last_time_;
};

/*
 * [
 *   {
 *     "id": int,
 *     "timestamp": "ISO",
 *     "points": [
 *        [x, y, td],
 *        [x, y, td],
 *     ]
 *   },
 *   {
 *     "id": int,
 *     "timestamp": "ISO",
 *     "points": [
 *        [x, y, td],
 *        [x, y, td],
 *     ]
 *   }
 * ]
 */

} // namespace processors
} // namespace connect
} // namespace prism

#endif // PRISM_CONNECT_PROCESSORS_Track_H_
