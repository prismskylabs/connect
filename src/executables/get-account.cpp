#include <iostream>

#include "api/client.h"
#include "api/environment.h"

using namespace prism::connect::api;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: get-account [account_id]" << std::endl;
        return -1;
    }

    Client client{environment::ApiRoot(), environment::ApiToken()};
    
    auto account = client.QueryAccount(std::stoul(argv[1]));

    std::cout << "Account[" << account.id_ << "]:" << std::endl;
    std::cout << "\tName: " << account.name_ << std::endl;
    std::cout << "\tUrl: " << account.url_ << std::endl;
    std::cout << "\tInstruments Url: " << account.instruments_url_ << std::endl;
    std::cout << std::endl;
}
