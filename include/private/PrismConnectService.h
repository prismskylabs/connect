/*
 * Copyright (C) 2017 Prism Skylabs
 */
#ifndef PRISM_PRISMCONNECTSERVICE_H
#define PRISM_PRISMCONNECTSERVICE_H

#include <client.h>
#include <boost/shared_ptr.hpp>

namespace prism
{
namespace connect
{

class PrismConnectService
{
public:

    struct Configuration
    {
        std::string apiRoot;
        std::string apiToken;
        std::string cameraName;
        std::string logLevel;
    };

    PrismConnectService() {}
    virtual ~PrismConnectService() {}

    int init(const Configuration& cfg);

    boost::shared_ptr<prism::connect::Client> client;

    id_t accountId;
    id_t instrumentId;
};

typedef boost::shared_ptr<PrismConnectService> PrismConnectServicePtr;

} // namespace connect
} // namespace prism

#endif // PRISMCONNECTSERVICE_H
