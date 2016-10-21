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

    nlohmann::json QueryInstrumentConfiguration(const Instrument& instrument);
    bool EchoInstrument(const Instrument& instrument);

    // Generic POST methods
    bool PostImage(const Instrument& instrument, const std::string& key,
                   const std::chrono::system_clock::time_point& timestamp,
                   const std::chrono::system_clock::time_point& event_timestamp,
                   const std::string& image_name, const std::vector<char>& image_data);
    bool PostVideo(const Instrument& instrument, const std::string& key,
                   const std::chrono::system_clock::time_point& timestamp,
                   const std::chrono::system_clock::time_point& event_timestamp,
                   const std::string& video_name, const std::vector<char>& video_data);
    bool PostTimeSeries(const Instrument& instrument, const std::string& key,
                        const std::chrono::system_clock::time_point& timestamp,
                        const nlohmann::json& json_data);

    // Reserved POST methods
    bool PostImageBackground(const Instrument& instrument,
                             const std::chrono::system_clock::time_point& timestamp,
                             const std::string& image_name, const std::vector<char>& image_data);
    bool PostImageTapestry(const Instrument& instrument,
                           const std::chrono::system_clock::time_point& timestamp,
                           const std::chrono::system_clock::time_point& event_timestamp,
                           const std::string& image_name, const std::vector<char>& image_data);
    bool PostImageLiveTile(const Instrument& instrument,
                           const std::chrono::system_clock::time_point& timestamp,
                           const std::chrono::system_clock::time_point& event_timestamp,
                           const std::string& image_name, const std::vector<char>& image_data);
    bool PostVideoFull(const Instrument& instrument,
                       const std::chrono::system_clock::time_point& timestamp,
                       const std::chrono::system_clock::time_point& event_timestamp,
                       const std::string& video_name, const std::vector<char>& video_data);
    bool PostVideoLiveLoop(const Instrument& instrument,
                           const std::chrono::system_clock::time_point& timestamp,
                           const std::chrono::system_clock::time_point& event_timestamp,
                           const std::string& video_name, const std::vector<char>& video_data);
    bool PostVideoFlipbook(const Instrument& instrument,
                           const std::chrono::system_clock::time_point& timestamp,
                           const std::chrono::system_clock::time_point& event_timestamp,
                           const std::string& video_name, const std::vector<char>& video_data);
    bool PostTimeSeriesCounts(const Instrument& instrument,
                              const std::chrono::system_clock::time_point& timestamp,
                              const nlohmann::json& json_data);
    bool PostTimeSeriesEvents(const Instrument& instrument,
                              const std::chrono::system_clock::time_point& timestamp,
                              const nlohmann::json& json_data);

  private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace api
} // namespace connect
} // namespace prism

#endif // PRISM_CONNECT_API_Client_H_
