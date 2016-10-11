#ifndef PRISM_CONNECT_API_Client_H_
#define PRISM_CONNECT_API_Client_H_

#include <memory>
#include <string>
#include <vector>

#include "api/account.h"
#include "api/instrument.h"

namespace prism {
namespace connect {
namespace api {

class Client {
  public:
    Client(const std::string& api_root, const std::string& api_token);
    ~Client();

    std::vector<Account> QueryAccounts();
    Account QueryAccount(const std::uint32_t id);

    std::vector<Instrument> QueryInstruments(const Account& account);
    Instrument QueryInstrument(const Account& account, const std::uint32_t id);

  private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace api
} // namespace connect
} // namespace prism

#endif // PRISM_CONNECT_API_Client_H_
