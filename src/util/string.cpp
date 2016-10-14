#include "util/string.h"

#include <string>
#include <unordered_map>

namespace prism {
namespace connect {
namespace util {

static const std::unordered_map<std::string, std::string> mime_mapping{{"png", "image/png"},
                                                                       {"jpg", "image/jpeg"},
                                                                       {"jpeg", "image/jpeg"},
                                                                       {"mp4", "video/mp4"},
                                                                       {"h264", "video/h264"}};

std::string ParseMimeType(const std::string& file_path) {
    auto extension_character = file_path.find_last_of('.');

    if (extension_character == std::string::npos) {
        return "";
    }

    auto lookup = mime_mapping.find(file_path.substr(extension_character + 1));

    if (lookup == mime_mapping.end()) {
        return "";
    }

    return lookup->second;
}

} // namespace util
} // namespace connect
} // namespace prism
