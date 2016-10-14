#ifndef PRISM_CONNECT_SOURCES_FileSource_H_
#define PRISM_CONNECT_SOURCES_FileSource_H_

#include "sources/source-interface.h"

#include <fstream>
#include <string>

namespace prism {
namespace connect {
namespace sources {

class FileSource : public SourceInterface {
  public:
    FileSource(const std::string& file_path);

    virtual FramePtr GetFrame() override;

  private:
    void freeFrame(Frame* frame);

    std::ifstream input_file_;
};

} // namespace sources
} // namespace connect
} // namespace prism

#endif // PRISM_CONNECT_SOURCES_FileSource_H_
