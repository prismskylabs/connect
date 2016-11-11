#include "api/client.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include <cpr/cpr.h>
#include <fmt/format.h>
#include <json.hpp>

#include "api/account.h"
#include "api/instrument.h"
#include "api/response.h"
#include "util/string.h"
#include "util/time.h"

namespace prism {
namespace connect {
namespace api {

class Client::Impl {
  public:
    Impl(const std::string& api_root, const std::string& api_token);

    std::vector<Account> QueryAccounts();
    Account QueryAccount(const std::uint32_t id);

    std::vector<Instrument> QueryInstruments(const Account& account);
    Instrument QueryInstrument(const Account& account, const std::uint32_t id);
    Response RegisterInstrument(const Account& account, const Instrument& instrument);

    nlohmann::json QueryInstrumentConfiguration(const Instrument& instrument);
    Response EchoInstrument(const Instrument& instrument);

    Response PostMultipart(const Instrument& instrument, const cpr::Url& url,
                           const cpr::Multipart& multipart);

    Response PostImage(const Instrument& instrument, const std::string& key,
                       const std::chrono::system_clock::time_point& timestamp,
                       const std::chrono::system_clock::time_point& event_timestamp,
                       const std::string& image_name, const std::vector<char>& image_data);
    Response PostImageFile(const Instrument& instrument, const std::string& key,
                           const std::chrono::system_clock::time_point& timestamp,
                           const std::chrono::system_clock::time_point& event_timestamp,
                           const std::string& image_path);
    Response PostVideo(const Instrument& instrument, const std::string& key,
                       const std::chrono::system_clock::time_point& timestamp,
                       const std::chrono::system_clock::time_point& event_timestamp,
                       const std::string& video_name, const std::vector<char>& video_data);
    Response PostVideoFile(const Instrument& instrument, const std::string& key,
                           const std::chrono::system_clock::time_point& start_timestamp,
                           const std::chrono::system_clock::time_point& stop_timestamp,
                           const std::string& video_path);
    Response PostTimeSeries(const Instrument& instrument, const std::string& key,
                            const std::chrono::system_clock::time_point& timestamp,
                            const nlohmann::json& json_data);

  private:
    std::string api_root_;
    std::string api_token_;
    cpr::Session session_;
    cpr::Header headers_;

    std::string version_;
    std::string accounts_url_;
};

Client::Impl::Impl(const std::string& api_root, const std::string& api_token)
        : api_root_{api_root}, api_token_{api_token} {
    headers_["Authorization"] = std::string{"Token "} + api_token_;
    session_.SetUrl(api_root);
    session_.SetHeader(headers_);

    auto response = session_.Get();

    if (response.error) {
        throw std::runtime_error(fmt::format("CPR error[{}]: {}",
                                             static_cast<int>(response.error.code),
                                             response.error.message));
    }

    if (response.status_code != 200) {
        throw std::runtime_error(
                fmt::format("Http error[{}]: {}", response.status_code, response.text));
    } else {
        auto response_json = nlohmann::json::parse(response.text);
        version_ = response_json["version"].get<std::string>();
        accounts_url_ = response_json["accounts_url"].get<std::string>();
    }

    if (accounts_url_.empty()) {
        throw std::runtime_error(
                fmt::format("Client could not establish a connection to {}", api_root));
    }
}

std::vector<Account> Client::Impl::QueryAccounts() {
    std::vector<Account> accounts;

    session_.SetUrl(accounts_url_);
    auto response = session_.Get();

    if (!response.error && response.status_code == 200) {
        auto response_json = nlohmann::json::parse(response.text);

        for (const auto& account_json : response_json) {
            accounts.emplace_back(account_json);
        }
    }

    return accounts;
}

Account Client::Impl::QueryAccount(const std::uint32_t id) {
    session_.SetUrl(accounts_url_ + std::to_string(id) + "/");
    auto response = session_.Get();

    if (!response.error && response.status_code == 200) {
        return Account{nlohmann::json::parse(response.text)};
    }

    return Account{};
}

std::vector<Instrument> Client::Impl::QueryInstruments(const Account& account) {
    if (!account) {
        throw std::runtime_error("Cannot query Instruments of an invalid Account");
    }

    std::vector<Instrument> instruments;

    session_.SetUrl(account.instruments_url_);
    auto response = session_.Get();

    if (!response.error && response.status_code == 200) {
        auto response_json = nlohmann::json::parse(response.text);

        for (const auto& instrument_json : response_json) {
            instruments.emplace_back(instrument_json);
            auto& instrument = instruments.back();
            instrument.url_ = account.instruments_url_ + std::to_string(instrument.id_);
        }
    }

    return instruments;
}

Instrument Client::Impl::QueryInstrument(const Account& account, const std::uint32_t id) {
    if (!account) {
        throw std::runtime_error("Cannot query Instrument of an invalid Account");
    }

    session_.SetUrl(account.instruments_url_ + std::to_string(id) + "/");
    auto response = session_.Get();

    if (!response.error && response.status_code == 200) {
        auto instrument = Instrument{nlohmann::json::parse(response.text)};
        instrument.url_ = account.instruments_url_ + std::to_string(instrument.id_);
        return instrument;
    }

    return Instrument{};
}

Response Client::Impl::RegisterInstrument(const Account& account, const Instrument& instrument) {
    if (!account) {
        throw std::runtime_error("Cannot register Instrument to an invalid Account");
    }

    auto instrument_json = instrument.ToJson();

    if (instrument_json.empty()) {
        throw std::runtime_error("Cannot register empty Instrument to Account");
    }

    headers_["Content-Type"] = "application/json";
    session_.SetUrl(account.instruments_url_);
    session_.SetBody(cpr::Body{instrument_json.dump()});
    session_.SetHeader(headers_);
    auto response = session_.Post();

    return {response.status_code, response.text};
}

nlohmann::json Client::Impl::QueryInstrumentConfiguration(const Instrument& instrument) {
    if (!instrument) {
        throw std::runtime_error("Cannot query invalid Instrument configuration");
    }

    session_.SetUrl(instrument.url_ + "/configuration/");
    auto response = session_.Get();

    if (!response.error && response.status_code == 200) {
        return nlohmann::json::parse(response.text);
    }

    return nlohmann::json{};
}

Response Client::Impl::EchoInstrument(const Instrument& instrument) {
    if (!instrument) {
        throw std::runtime_error("Cannot echo to an invalid Instrument");
    }

    session_.SetUrl(instrument.url_ + "/echo/");
    session_.SetMultipart({});
    auto response = session_.Post();

    return {response.status_code, response.text};
}

Response Client::Impl::PostMultipart(const Instrument& instrument, const cpr::Url& url,
                                     const cpr::Multipart& multipart) {
    if (!instrument) {
        throw std::runtime_error("Cannot POST form to an invalid Instrument");
    }

    session_.SetUrl(url);
    session_.SetMultipart(multipart);
    auto response = session_.Post();

    return {response.status_code, response.text};
}

Response Client::Impl::PostImage(const Instrument& instrument, const std::string& key,
                                 const std::chrono::system_clock::time_point& timestamp,
                                 const std::chrono::system_clock::time_point& event_timestamp,
                                 const std::string& image_name,
                                 const std::vector<char>& image_data) {
    if (!instrument) {
        throw std::runtime_error("Cannot POST image to an invalid Instrument");
    }

    session_.SetUrl(instrument.url_ + "/data/images/");
    session_.SetMultipart({{"key", key},
                           {"timestamp", util::IsoTime(timestamp)},
                           {"event_timestamp", util::IsoTime(event_timestamp)},
                           {"data", image_data.data(), util::ParseMimeType(image_name)}});
    auto response = session_.Post();

    return {response.status_code, response.text};
}

Response Client::Impl::PostImageFile(const Instrument& instrument, const std::string& key,
                                     const std::chrono::system_clock::time_point& timestamp,
                                     const std::chrono::system_clock::time_point& event_timestamp,
                                     const std::string& image_path) {
    if (!instrument) {
        throw std::runtime_error("Cannot POST image to an invalid Instrument");
    }

    session_.SetUrl(instrument.url_ + "/data/images/");
    session_.SetMultipart({{"key", key},
                           {"timestamp", util::IsoTime(timestamp)},
                           {"event_timestamp", util::IsoTime(event_timestamp)},
                           {"data", cpr::File{image_path}, util::ParseMimeType(image_path)}});
    auto response = session_.Post();

    return {response.status_code, response.text};
}

Response Client::Impl::PostVideo(const Instrument& instrument, const std::string& key,
                                 const std::chrono::system_clock::time_point& start_timestamp,
                                 const std::chrono::system_clock::time_point& stop_timestamp,
                                 const std::string& video_name,
                                 const std::vector<char>& video_data) {
    if (!instrument) {
        throw std::runtime_error("Cannot POST video to an invalid Instrument");
    }

    session_.SetUrl(instrument.url_ + "/data/videos/");
    session_.SetMultipart({{"key", key},
                           {"start_timestamp", util::IsoTime(start_timestamp)},
                           {"stop_timestamp", util::IsoTime(stop_timestamp)},
                           {"data", video_data.data(), util::ParseMimeType(video_name)}});
    auto response = session_.Post();

    return {response.status_code, response.text};
}

Response Client::Impl::PostVideoFile(const Instrument& instrument, const std::string& key,
                                     const std::chrono::system_clock::time_point& start_timestamp,
                                     const std::chrono::system_clock::time_point& stop_timestamp,
                                     const std::string& video_path) {
    if (!instrument) {
        throw std::runtime_error("Cannot POST video to an invalid Instrument");
    }

    session_.SetUrl(instrument.url_ + "/data/videos/");
    session_.SetMultipart({{"key", key},
                           {"start_timestamp", util::IsoTime(start_timestamp)},
                           {"stop_timestamp", util::IsoTime(stop_timestamp)},
                           {"data", cpr::File{video_path}, util::ParseMimeType(video_path)}});
    auto response = session_.Post();

    return {response.status_code, response.text};
}

Response Client::Impl::PostTimeSeries(const Instrument& instrument, const std::string& key,
                                      const std::chrono::system_clock::time_point& timestamp,
                                      const nlohmann::json& json_data) {
    return PostMultipart(instrument, instrument.url_ + "/data/time-series/",
                         {{"key", key},
                          {"timestamp", util::IsoTime(timestamp)},
                          {"data", json_data.dump(), "application/json"}});
}

Client::Client(const std::string& api_root, const std::string& api_token)
        : pimpl_{new Impl{api_root, api_token}} {}

Client::~Client() {}

std::vector<Account> Client::QueryAccounts() {
    return pimpl_->QueryAccounts();
}

Account Client::QueryAccount(const std::uint32_t id) {
    return pimpl_->QueryAccount(id);
}

std::vector<Instrument> Client::QueryInstruments(const Account& account) {
    return pimpl_->QueryInstruments(account);
}

Instrument Client::QueryInstrument(const Account& account, const std::uint32_t id) {
    return pimpl_->QueryInstrument(account, id);
}

Response Client::RegisterInstrument(const Account& account, const Instrument& instrument) {
    return pimpl_->RegisterInstrument(account, instrument);
}

nlohmann::json Client::QueryInstrumentConfiguration(const Instrument& instrument) {
    return pimpl_->QueryInstrumentConfiguration(instrument);
}

Response Client::EchoInstrument(const Instrument& instrument) {
    return pimpl_->EchoInstrument(instrument);
}

Response Client::PostImage(const Instrument& instrument, const std::string& key,
                           const std::chrono::system_clock::time_point& timestamp,
                           const std::chrono::system_clock::time_point& event_timestamp,
                           const std::string& image_name, const std::vector<char>& image_data) {
    return pimpl_->PostImage(instrument, key, timestamp, event_timestamp, image_name, image_data);
}

Response Client::PostImageFile(const Instrument& instrument, const std::string& key,
                               const std::chrono::system_clock::time_point& timestamp,
                               const std::chrono::system_clock::time_point& event_timestamp,
                               const std::string& image_path) {
    return pimpl_->PostImageFile(instrument, key, timestamp, event_timestamp, image_path);
}

Response Client::PostVideo(const Instrument& instrument, const std::string& key,
                           const std::chrono::system_clock::time_point& start_timestamp,
                           const std::chrono::system_clock::time_point& stop_timestamp,
                           const std::string& video_name, const std::vector<char>& video_data) {
    return pimpl_->PostVideo(instrument, key, start_timestamp, stop_timestamp, video_name,
                             video_data);
}

Response Client::PostVideoFile(const Instrument& instrument, const std::string& key,
                               const std::chrono::system_clock::time_point& start_timestamp,
                               const std::chrono::system_clock::time_point& stop_timestamp,
                               const std::string& video_path) {
    return pimpl_->PostVideoFile(instrument, key, start_timestamp, stop_timestamp, video_path);
}

Response Client::PostTimeSeries(const Instrument& instrument, const std::string& key,
                                const std::chrono::system_clock::time_point& timestamp,
                                const nlohmann::json& json_data) {
    return pimpl_->PostTimeSeries(instrument, key, timestamp, json_data);
}

Response Client::PostImageBackground(const Instrument& instrument,
                                     const std::chrono::system_clock::time_point& timestamp,
                                     const std::string& image_name,
                                     const std::vector<char>& image_data) {
    return pimpl_->PostMultipart(instrument, instrument.url_ + "/data/images/",
                                 {{"key", "BACKGROUND"},
                                  {"timestamp", util::IsoTime(timestamp)},
                                  {"data", image_data.data(), util::ParseMimeType(image_name)}});
}

Response Client::PostImageTapestry(const Instrument& instrument, const std::string& type,
                                   const std::chrono::system_clock::time_point& event_timestamp,
                                   const std::string& image_name,
                                   const std::vector<char>& image_data) {
    return pimpl_->PostMultipart(instrument, instrument.url_ + "/data/images/",
                                 {{"key", "TAPESTRY"},
                                  {"type", type},
                                  {"event_timestamp", util::IsoTime(event_timestamp)},
                                  {"data", image_data.data(), util::ParseMimeType(image_name)}});
}

Response Client::PostImageLiveTile(const Instrument& instrument,
                                   const std::chrono::system_clock::time_point& event_timestamp,
                                   const std::string& image_name,
                                   const std::vector<char>& image_data) {
    return pimpl_->PostMultipart(instrument, instrument.url_ + "/data/images/",
                                 {{"key", "LIVETILE"},
                                  {"event_timestamp", util::IsoTime(event_timestamp)},
                                  {"data", image_data.data(), util::ParseMimeType(image_name)}});
}

Response Client::PostImageFileBackground(const Instrument& instrument,
                                         const std::chrono::system_clock::time_point& timestamp,
                                         const std::string& image_path) {
    return pimpl_->PostMultipart(
            instrument, instrument.url_ + "/data/images/",
            {{"key", "BACKGROUND"},
             {"timestamp", util::IsoTime(timestamp)},
             {"data", cpr::File{image_path}, util::ParseMimeType(image_path)}});
}

Response Client::PostImageFileTapestry(const Instrument& instrument, const std::string& type,
                                       const std::chrono::system_clock::time_point& event_timestamp,
                                       const std::string& image_path) {
    return pimpl_->PostMultipart(
            instrument, instrument.url_ + "/data/images/",
            {{"key", "TAPESTRY"},
             {"type", type},
             {"event_timestamp", util::IsoTime(event_timestamp)},
             {"data", cpr::File{image_path}, util::ParseMimeType(image_path)}});
}

Response Client::PostImageFileLiveTile(const Instrument& instrument,
                                       const std::chrono::system_clock::time_point& event_timestamp,
                                       const std::string& image_path) {
    return pimpl_->PostMultipart(
            instrument, instrument.url_ + "/data/images/",
            {{"key", "LIVETILE"},
             {"event_timestamp", util::IsoTime(event_timestamp)},
             {"data", cpr::File{image_path}, util::ParseMimeType(image_path)}});
}

Response Client::PostVideoFull(const Instrument& instrument,
                               const std::chrono::system_clock::time_point& start_timestamp,
                               const std::chrono::system_clock::time_point& stop_timestamp,
                               const std::string& video_name, const std::vector<char>& video_data) {
    return pimpl_->PostVideo(instrument, "VIDEO", start_timestamp, stop_timestamp, video_name,
                             video_data);
}

Response Client::PostVideoLiveLoop(const Instrument& instrument,
                                   const std::chrono::system_clock::time_point& start_timestamp,
                                   const std::chrono::system_clock::time_point& stop_timestamp,
                                   const std::string& video_name,
                                   const std::vector<char>& video_data) {
    return pimpl_->PostVideo(instrument, "LIVELOOP", start_timestamp, stop_timestamp, video_name,
                             video_data);
}

Response Client::PostVideoFlipbook(const Instrument& instrument,
                                   const std::chrono::system_clock::time_point& start_timestamp,
                                   const std::chrono::system_clock::time_point& stop_timestamp,
                                   const std::string& video_name,
                                   const std::vector<char>& video_data) {
    return pimpl_->PostVideo(instrument, "FLIPBOOK", start_timestamp, stop_timestamp, video_name,
                             video_data);
}

Response Client::PostVideoFileFull(const Instrument& instrument,
                                   const std::chrono::system_clock::time_point& start_timestamp,
                                   const std::chrono::system_clock::time_point& stop_timestamp,
                                   const std::string& video_path) {
    return pimpl_->PostVideoFile(instrument, "VIDEO", start_timestamp, stop_timestamp, video_path);
}

Response Client::PostVideoFileLiveLoop(const Instrument& instrument,
                                       const std::chrono::system_clock::time_point& start_timestamp,
                                       const std::chrono::system_clock::time_point& stop_timestamp,
                                       const std::string& video_path) {
    return pimpl_->PostVideoFile(instrument, "LIVELOOP", start_timestamp, stop_timestamp,
                                 video_path);
}

Response Client::PostVideoFileFlipbook(const Instrument& instrument,
                                       const std::chrono::system_clock::time_point& start_timestamp,
                                       const std::chrono::system_clock::time_point& stop_timestamp,
                                       const std::string& video_path) {
    return pimpl_->PostVideoFile(instrument, "FLIPBOOK", start_timestamp, stop_timestamp,
                                 video_path);
}

Response Client::PostTimeSeriesCounts(const Instrument& instrument,
                                      const std::chrono::system_clock::time_point& timestamp,
                                      const nlohmann::json& json_data) {
    return pimpl_->PostTimeSeries(instrument, "COUNT", timestamp, json_data);
}

Response Client::PostTimeSeriesEvents(const Instrument& instrument,
                                      const std::chrono::system_clock::time_point& timestamp,
                                      const nlohmann::json& json_data) {
    return pimpl_->PostTimeSeries(instrument, "EVENT", timestamp, json_data);
}

Response Client::PostTimeSeriesTracks(const Instrument& instrument,
                                      const std::chrono::system_clock::time_point& timestamp,
                                      const nlohmann::json& json_data) {
    return pimpl_->PostTimeSeries(instrument, "TRACK", timestamp, json_data);
}

} // namespace api
} // namespace connect
} // namespace prism
