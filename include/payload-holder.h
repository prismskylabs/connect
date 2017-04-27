/*
 * Copyright (C) 2017 Prism Skylabs
 */
#ifndef CONNECT_SDK_PAYLOAD_HOLDER_H
#define CONNECT_SDK_PAYLOAD_HOLDER_H

#include "boost/shared_ptr.hpp"
#include "domain-types.h"
#include "public-util.h"

namespace prism
{
namespace connect
{

class PayloadHolder;
typedef boost::shared_ptr<PayloadHolder> PayloadHolderPtr;

// This class is carefully designed to take ownership and clean data after use.
// The moment, when data is copied or stolen is well-defined: in a call
// to make* function.
// The moment, when owned data or file is deleted is uknknown: whenever
// PayloadHolder instance is destroyed.
// Copying and assignment are explicitly prohibited to prevent unintended
// losing of ownership or data copying.
// This class is intentionally made distinct from Payload struct to keep
// current interface using Payload unchanged.
class PayloadHolder : private boost::noncopyable
{
public:

    bool isFile() const;
    std::string getFilePath() const;
    std::string getMimeType() const;
    const uint8_t* getData() const;
    size_t getDataSize() const;

private:
    friend PayloadHolderPtr makePayloadHolderByMovingData(move_ref<ByteBuffer> data, const std::string& mimeType);
    friend PayloadHolderPtr makePayloadHolderByCopyingData(const void* data, size_t dataSize, const std::string& mimeType);
    friend PayloadHolderPtr makePayloadHolderByReferencingFileAutodelete(const std::string& filePath);

    PayloadHolder();

    class Impl;
    unique_ptr<Impl>::t pImpl_;

    // using this instead of pImpl_-> enables autocomplete and go to definition in QtCreator
    Impl& impl() const
    {
        return *pImpl_;
    }
};

PayloadHolderPtr makePayloadHolderByMovingData(move_ref<ByteBuffer> data, const std::string& mimeType);
PayloadHolderPtr makePayloadHolderByCopyingData(const void* data, size_t dataSize, const std::string& mimeType);

// file will be deleted on PayloadHolder destruction
PayloadHolderPtr makePayloadHolderByReferencingFileAutodelete(const std::string& filePath);

} // namespace connect
} // namespace prism

#endif // CONNECT_SDK_PAYLOAD_HOLDER_H
