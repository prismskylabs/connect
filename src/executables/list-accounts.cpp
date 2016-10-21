#include <iostream>

#include "api/client.h"
#include "api/environment.h"

using namespace prism::connect::api;

int main() {
    Client client{environment::ApiRoot(), environment::ApiToken()};
    
    auto accounts = client.QueryAccounts();

    for (const auto& account : accounts) {
        std::cout << "Account[" << account.id_ << "]:" << std::endl;
        std::cout << "\tName: " << account.name_ << std::endl;
        std::cout << "\tUrl: " << account.url_ << std::endl;
        std::cout << "\tInstruments Url: " << account.instruments_url_ << std::endl;
        std::cout << std::endl;
    }
}
