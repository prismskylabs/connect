#ifndef PRISM_CONNECT_SOURCES_SourceInterface_H_
#define PRISM_CONNECT_SOURCES_SourceInterface_H_

#include <functional>
#include <memory>

namespace prism {
namespace connect {
namespace sources {

struct Frame {
    void* data;
    int data_size;
    double epoch_time;
};

using FramePtr = std::unique_ptr<Frame, std::function<void(Frame*)>>;

class SourceInterface {
  public:
    virtual ~SourceInterface() {}

    virtual FramePtr GetFrame() = 0;
};

} // namespace sources
} // namespace connect
} // namespace prism

#endif // PRISM_CONNECT_SOURCES_SourceInterface_H_
