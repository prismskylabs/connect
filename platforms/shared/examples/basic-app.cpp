/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "client.h"
#include "curl/curl.h"
#include "easylogging++.h"

_INITIALIZE_EASYLOGGINGPP

namespace prc = prism::connect;

struct CurlGlobal
{
    CurlGlobal()
    {
        curl_global_init(CURL_GLOBAL_ALL);
    }

    ~CurlGlobal()
    {
        curl_global_cleanup();
    }
};

void initLogger()
{
    namespace el = easyloggingpp;

    el::Configurations conf;

    conf.set(el::Level::All, el::ConfigurationType::Format, "%datetime | %level | %log");
    conf.set(el::Level::All, el::ConfigurationType::ToFile, "false");
    conf.set(el::Level::All, el::ConfigurationType::Enabled, "true");

    el::Loggers::reconfigureAllLoggers(conf);
}

int main(int argc, char** argv)
{
    initLogger();

    const std::string apiRoot = "<API root>";
    const std::string apiToken = "<API token>";
    const std::string cameraName = "<camera name>";

    CurlGlobal cg;

    prc::Client client(apiRoot, apiToken);

    // Configure client e.g. enable debug logging, set network timeouts,
    // set path to certificates bundle
    client.setCaBundlePath("<path to certificates file>");
    client.setConnectionTimeoutMs(5000);
    client.setLogFlags(prc::Client::LOG_INPUT | prc::Client::LOG_INPUT_JSON);
    client.setLowSpeed(5);

    // Successful init() means we are OK with API root, token, certificates
    prc::Status status = client.init();

    if (status.isError())
    {
        LOG(ERROR) << "init() failed";
        return -1;
    }

    prc::Accounts accounts;
    status = client.queryAccountsList(accounts);

    if (status.isError()  ||  accounts.empty())
    {
        LOG(ERROR) << "No accounts associated with given token, exiting";
        return -1;
    }

    int accountId = accounts[0].id;

    LOG(INFO) << "Account ID: " << accountId;

    prc::Instrument instrument;

    if (prc::findCameraByName(client, accountId, cameraName, instrument).isError())
    {
        status = prc::registerNewCamera(client, accountId, cameraName, instrument);

        if (status.isError())
        {
            LOG(ERROR) << "Failed to create camera: " << cameraName;
            return -1;
        }
    }

    int instrumentId = instrument.id;

    LOG(INFO) << "Instrument ID: " << instrumentId;

    return 0;
}
