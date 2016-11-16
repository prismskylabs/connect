#ifndef PRISM_CONNECT_API_Client_H_
#define PRISM_CONNECT_API_Client_H_

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "api/account.h"
#include "api/instrument.h"
#include "api/response.h"

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
    Response RegisterInstrument(const Account& account, const Instrument& instrument);

    nlohmann::json QueryInstrumentConfiguration(const Instrument& instrument);
    Response EchoInstrument(const Instrument& instrument);

    // Generic POST methods
    Response PostImage(const Instrument& instrument, const std::string& key,
                       const std::chrono::system_clock::time_point& timestamp,
                       const std::chrono::system_clock::time_point& event_timestamp,
                       const std::string& image_name, const std::vector<char>& image_data);
    Response PostImageFile(const Instrument& instrument, const std::string& key,
                           const std::chrono::system_clock::time_point& timestamp,
                           const std::chrono::system_clock::time_point& event_timestamp,
                           const std::string& image_path);
    Response PostVideo(const Instrument& instrument, const std::string& key,
                       const std::chrono::system_clock::time_point& start_timestamp,
                       const std::chrono::system_clock::time_point& stop_timestamp,
                       const std::string& video_name, const std::vector<char>& video_data);
    Response PostVideoFile(const Instrument& instrument, const std::string& key,
                           const std::chrono::system_clock::time_point& start_timestamp,
                           const std::chrono::system_clock::time_point& stop_timestamp,
                           const std::string& video_path);
    Response PostTimeSeries(const Instrument& instrument, const std::string& key,
                            const std::chrono::system_clock::time_point& timestamp,
                            const nlohmann::json& json_data);

    // Reserved POST methods
    Response PostImageBackground(const Instrument& instrument,
                                 const std::chrono::system_clock::time_point& timestamp,
                                 const std::string& image_name,
                                 const std::vector<char>& image_data);
    Response PostImageTapestry(const Instrument& instrument, const std::string& type,
                               const std::chrono::system_clock::time_point& event_timestamp,
                               const std::string& image_name, const std::vector<char>& image_data);
    Response PostImageLiveTile(const Instrument& instrument,
                               const std::chrono::system_clock::time_point& event_timestamp,
                               const std::string& image_name, const std::vector<char>& image_data);
    Response PostImageObjectStream(const Instrument& instrument, const nlohmann::json& meta_data,
                                   const std::string& image_name,
                                   const std::vector<char>& image_data);
    Response PostImageFileBackground(const Instrument& instrument,
                                     const std::chrono::system_clock::time_point& timestamp,
                                     const std::string& image_path);
    Response PostImageFileTapestry(const Instrument& instrument, const std::string& type,
                                   const std::chrono::system_clock::time_point& event_timestamp,
                                   const std::string& image_path);
    Response PostImageFileLiveTile(const Instrument& instrument,
                                   const std::chrono::system_clock::time_point& event_timestamp,
                                   const std::string& image_path);
    Response PostImageFileObjectStream(const Instrument& instrument,
                                       const nlohmann::json& meta_data,
                                       const std::string& image_path);
    Response PostVideoFull(const Instrument& instrument,
                           const std::chrono::system_clock::time_point& start_timestamp,
                           const std::chrono::system_clock::time_point& stop_timestamp,
                           const std::string& video_name, const std::vector<char>& video_data);
    Response PostVideoLiveLoop(const Instrument& instrument,
                               const std::chrono::system_clock::time_point& start_timestamp,
                               const std::chrono::system_clock::time_point& stop_timestamp,
                               const std::string& video_name, const std::vector<char>& video_data);
    Response PostVideoFlipbook(const Instrument& instrument,
                               const std::chrono::system_clock::time_point& start_timestamp,
                               const std::chrono::system_clock::time_point& stop_timestamp,
                               const std::string& video_name, const std::vector<char>& video_data);
    Response PostVideoFileFull(const Instrument& instrument,
                               const std::chrono::system_clock::time_point& start_timestamp,
                               const std::chrono::system_clock::time_point& stop_timestamp,
                               const std::string& video_path);
    Response PostVideoFileLiveLoop(const Instrument& instrument,
                                   const std::chrono::system_clock::time_point& start_timestamp,
                                   const std::chrono::system_clock::time_point& stop_timestamp,
                                   const std::string& video_path);
    Response PostVideoFileFlipbook(const Instrument& instrument,
                                   const std::chrono::system_clock::time_point& start_timestamp,
                                   const std::chrono::system_clock::time_point& stop_timestamp,
                                   const std::string& video_path);
    Response PostTimeSeriesCounts(const Instrument& instrument,
                                  const std::chrono::system_clock::time_point& timestamp,
                                  const nlohmann::json& json_data);
    Response PostTimeSeriesEvents(const Instrument& instrument,
                                  const std::chrono::system_clock::time_point& timestamp,
                                  const nlohmann::json& json_data);
    Response PostTimeSeriesTracks(const Instrument& instrument,
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
