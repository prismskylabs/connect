#ifndef PRISM_CONNECT_API_Client_H_
#define PRISM_CONNECT_API_Client_H_

#include <chrono>
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
    bool RegisterInstrument(const Account& account, const Instrument& instrument);

    bool PostImage(const Instrument& instrument, const std::string& key,
                   const std::chrono::system_clock::time_point& timestamp,
                   const std::chrono::system_clock::time_point& event_timestamp,
                   const std::string& image_name, const std::vector<char>& image_data);
    bool PostVideo();
    bool PostTimeSeries();

  private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace api
} // namespace connect
} // namespace prism

#endif // PRISM_CONNECT_API_Client_H_
