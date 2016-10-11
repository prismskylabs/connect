#include <iostream>

#include "api/client.h"
#include "api/environment.h"

using namespace prism::connect::api;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: list-instruments [account_id]" << std::endl;
        return -1;
    }

    Client client{environment::ApiRoot, environment::ApiToken};
    
    auto account = client.QueryAccount(std::stoul(argv[1]));
    auto instruments = client.QueryInstruments(account);

    for (const auto& instrument : instruments) {
        std::cout << "Instrument[" << instrument.id_ << "]:" << std::endl;
        std::cout << "\tName: " << instrument.name_ << std::endl;
        std::cout << std::endl;
    }
}
