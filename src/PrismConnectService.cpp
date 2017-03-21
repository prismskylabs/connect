/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "private/PrismConnectService.h"
#include <easylogging++.h>
#include "boost/format.hpp"

namespace prism
{
namespace connect
{

static bool findInstrumentByName(const boost::shared_ptr<Client>& client, id_t accountId,
                                 const std::string& cameraName, Instrument& instrument)
{
    Instruments instruments;
    Status status = client->queryInstrumentsList(accountId, instruments);

    if (status.isSuccess() && !instruments.empty())
        for (size_t i = 0; i < instruments.size(); ++i)
            if (instruments[i].name == cameraName)
            {
                instrument = instruments[i];
                return true;
            }

    return false;
}

int PrismConnectService::init(const Configuration& cfg)
{
    const std::string& cameraName = cfg.cameraName;

    client.reset(new Client(cfg.apiRoot, cfg.apiToken));

    Status status = client->init();

    LOG(INFO) << "client.init(): " << status.getCode();

    // Set SDK log level
    if (cfg.logLevel == "debug")
        client->setLogFlags(Client::LOG_INPUT | Client::LOG_INPUT_JSON);

    Accounts accounts;
    status = client->queryAccountsList(accounts);

    if (status.isError() || accounts.empty())
    {
        LOG(ERROR) << "No accounts associated with given token, exiting";
        return 1;
    }

    this->accountId = accounts[0].id;

    LOG(INFO) << "Account ID: " << accountId;

    Instrument instrument;

    if (!findInstrumentByName(client, accountId, cameraName, instrument))
    {
        Instrument newInstrument;
        newInstrument.name = cameraName;
        newInstrument.type = "camera";
        status = client->registerInstrument(accountId, newInstrument);

        if (status.isError())
        {
            LOG(ERROR) << (boost::format("Failed to register camera with name: %s") % cameraName).str();
            return 1;
        }

        if (!findInstrumentByName(client, accountId, cameraName, instrument))
        {
            LOG(ERROR) << (boost::format("Failed to find just registered instrument: %s") % cameraName).str();
            return 1;
        }
    }

    this->instrumentId = instrument.id;
    LOG(INFO) << "Instrument ID: " << instrumentId;

    return 0;
}

} // namespace connect
} // namespace prism
