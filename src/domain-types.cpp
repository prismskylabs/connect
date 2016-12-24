#include "domain-types.h"

namespace prism {
namespace connect {

class Metadata::Impl {
public:
    Impl() {}
//    ~Impl() {}

private:
    int i;
};

Metadata::Metadata()
    : pImpl_(new Impl())
{

}

Metadata::Metadata(const Metadata &)
{

}

Metadata::~Metadata()
{

}

}
}


