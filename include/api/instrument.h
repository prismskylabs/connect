#ifndef PRISM_CONNECT_API_Instrument_H_
#define PRISM_CONNECT_API_Instrument_H_

#include <cstdint>
#include <string>

#include <json.hpp>

namespace prism {
namespace connect {
namespace api {

class Instrument {
  public:
    Instrument();
    Instrument(const nlohmann::json& instrument_json);

    nlohmann::json ToJson() const;

    explicit operator bool() const;

    typedef struct {
        bool is_set = false;
        double latitude = 0.0;
        double longitude = 0.0;
    } GeoLocation;

    typedef struct {
        bool is_set = false;
        std::string country_code;
        std::string country_name;
        std::string city;
        std::string zip_code;
    } PhysicalAddress;

    // Required fields
    std::uint32_t id_ = 0;
    std::string name_;
    std::string url_;
    std::string instrument_type_;
    
    // Configuration fields
    std::string unique_id_;
    std::string platform_;
    std::string release_version_;
    std::string firmware_;
    std::string firmware_version_;
    std::string private_ip_;
    std::string public_ip_;
    std::string host_;
    std::string mac_address_;
    std::string timezone_;
    PhysicalAddress physical_address_;
    GeoLocation geo_location_;

    // Metadata fields
    std::string manufacturer_;
    std::string model_;
    std::uint32_t width_ = 0;
    std::uint32_t height_ = 0;
    std::uint32_t framerate_ = 0;
    nlohmann::json cpu_info_;
    nlohmann::json disk_info_;
    nlohmann::json memory_info_;
};

} // namespace api
} // namespace connect
} // namespace prism

#endif // PRISM_CONNECT_API_Instrument_H_
