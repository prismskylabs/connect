#ifndef PRISM_CONNECT_PROCESSORS_ObjectStream_H_
#define PRISM_CONNECT_PROCESSORS_ObjectStream_H_

#include <chrono>

#include <json.hpp>

namespace prism {
namespace connect {
namespace processors {

class ObjectStream {
  public:
    ObjectStream(int id, const std::chrono::system_clock::time_point& collected, const int x,
                 const int y, const int width, const int height, const int original_image_width,
                 const int original_image_height);

    nlohmann::json ToJson() const;

  private:
    nlohmann::json data_;
};

} // namespace processors
} // namespace connect
} // namespace prism

#endif // PRISM_CONNECT_PROCESSORS_ObjectStream_H_
