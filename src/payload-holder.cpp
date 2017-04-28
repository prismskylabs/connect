/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "payload-holder.h"

namespace prism
{
namespace connect
{

class PayloadHolder::Impl
{
public:
    ~Impl();

    bool isFile() const
    {
        return !filePath_.empty();
    }

    std::string getFilePath() const
    {
        return filePath_;
    }

    std::string getMimeType() const
    {
        return mimeType_;
    }

    const uint8_t* getData() const
    {
        return buf_.data();
    }

    size_t getDataSize() const
    {
        return buf_.size();
    }

private:
    friend PayloadHolderPtr makePayloadHolderByMovingData(move_ref<ByteBuffer> data, const std::string& mimeType);
    friend PayloadHolderPtr makePayloadHolderByCopyingData(const void* data, size_t dataSize, const std::string& mimeType);
    friend PayloadHolderPtr makePayloadHolderByReferencingFileAutodelete(const std::string& filePath);

    ByteBuffer buf_;
    std::string filePath_;
    std::string mimeType_;
};

PayloadHolder::PayloadHolder()
    : pImpl_(new Impl())
{
}

PayloadHolderPtr makePayloadHolderByMovingData(move_ref<ByteBuffer> data, const std::string& mimeType)
{
    PayloadHolderPtr rv(new PayloadHolder());
    std::swap(rv->impl().buf_, data.ref);
    rv->impl().mimeType_ = mimeType;

    return rv;
}

PayloadHolderPtr makePayloadHolderByCopyingData(const void* data, size_t dataSize, const std::string& mimeType)
{
    PayloadHolderPtr rv(new PayloadHolder());
    ByteBuffer& buf = rv->impl().buf_;
    buf.reserve(dataSize);
    const uint8_t* dataStart = static_cast<const uint8_t*>(data);
    buf.insert(buf.end(), dataStart, dataStart + dataSize);
    rv->impl().mimeType_ = mimeType;

    return rv;
}

PayloadHolderPtr makePayloadHolderByReferencingFileAutodelete(const std::string& filePath)
{
    PayloadHolderPtr rv(new PayloadHolder());
    rv->impl().filePath_ = filePath;

    return rv;
}

bool PayloadHolder::isFile() const
{
    return impl().isFile();
}

std::string PayloadHolder::getFilePath() const
{
    return impl().getFilePath();
}

std::string PayloadHolder::getMimeType() const
{
    return impl().getMimeType();
}

const uint8_t* PayloadHolder::getData() const
{
    return impl().getData();
}

size_t PayloadHolder::getDataSize() const
{
    return impl().getDataSize();
}

PayloadHolder::Impl::~Impl()
{
    if (isFile())
        removeFile(filePath_);
}

}
}
