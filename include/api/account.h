#ifndef PRISM_CONNECT_API_Account_H_
#define PRISM_CONNECT_API_Account_H_

#include <cstdint>
#include <string>
#include <vector>

#include "api/instrument.h"

namespace prism {
namespace connect {
namespace api {

class Account {
  public:
    static Account GetAccount(std::uint32_t id);
    static std::vector<Account> GetAccounts();

    std::vector<Instrument> GetInstruments();

  private:
    std::uint32_t id_;
    std::string name_;
    std::string url_;
    std::string instrument_url_;
};

} // namespace api
} // namespace connect
} // namespace prism

#endif // PRISM_CONNECT_API_Account_H_
