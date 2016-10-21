#include <iostream>

#include "api/client.h"
#include "api/environment.h"

using namespace prism::connect::api;

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: get-instrument [account_id] [instrument_id]" << std::endl;
        return -1;
    }

    Client client{environment::ApiRoot(), environment::ApiToken()};
    
    auto account = client.QueryAccount(std::stoul(argv[1]));
    auto instrument = client.QueryInstrument(account, std::stoul(argv[2]));

    std::cout << "Instrument[" << instrument.id_ << "]:" << std::endl;
    std::cout << "\tName: " << instrument.name_ << std::endl;
    std::cout << std::endl;

    if (client.EchoInstrument(instrument)) {
        std::cout << "Successfully sent echo for Instrument" << std::endl;
    } else {
        std::cerr << "Failed to send echo for Instrument" << std::endl;
        return -1;
    }

    return 0;
}
