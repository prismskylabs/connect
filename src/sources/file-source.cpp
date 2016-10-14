#include "sources/file-source.h"

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <string>

namespace prism {
namespace connect {
namespace sources {

FileSource::FileSource(const std::string& file_path) {
    input_file_.open(file_path, std::ifstream::binary);
}

FramePtr FileSource::GetFrame() {
    if (input_file_.is_open()) {
        input_file_.seekg(0, input_file_.end);
        int length = input_file_.tellg();
        if (length <= 0) {
            return nullptr;
        }
        input_file_.seekg(0, input_file_.beg);

        char* buffer = new char[length];
        input_file_.read(buffer, length);

        auto deleter = [this](Frame* frame) {
            this->freeFrame(frame);
        };
        auto frame = FramePtr(new Frame{}, deleter);

        frame->data = (void*) buffer;
        frame->data_size = length;
        auto epoch_time = std::chrono::system_clock::now().time_since_epoch();
        auto ns_since_epoch = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch_time);
        frame->epoch_time = ns_since_epoch.count() / 1e9;
        return std::move(frame);
    }

    return nullptr;
}

void FileSource::freeFrame(Frame* frame) {
    if (frame) {
        free(frame->data);
    }
    delete frame;
}

} // namespace sources
} // namespace connect
} // namespace prism
