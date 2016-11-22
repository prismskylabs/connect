#include "processors/object-stream.h"

#include <chrono>

#include <json.hpp>

#include "util/time.h"

namespace prism {
namespace connect {
namespace processors {

ObjectStream::ObjectStream(int id, const std::chrono::system_clock::time_point& collected,
                           const int x, const int y, const int width, const int height,
                           const int original_image_width, const int original_image_height)
        : data_(nlohmann::json::object()) {
    data_["object_id"] = id;
    data_["collected"] = util::IsoTime(collected);
    data_["location_x"] = x;
    data_["location_y"] = y;
    data_["width"] = width;
    data_["height"] = height;
    data_["orig_image_width"] = original_image_width;
    data_["orig_image_height"] = original_image_height;
    data_["stream_type"] = std::string{"foreground"}; // TODO: Add other stream_type's like mask
}

nlohmann::json ObjectStream::ToJson() const {
    return data_;
}

} // namespace processors
} // namespace connect
} // namespace prism
