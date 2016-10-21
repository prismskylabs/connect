#include "api/instrument.h"

#include <cstdint>
#include <string>

#include <json.hpp>

namespace prism {
namespace connect {
namespace api {

Instrument::Instrument() {}

Instrument::Instrument(const nlohmann::json& instrument_json)
        : id_{instrument_json["id"].get<std::uint32_t>()},
          name_{instrument_json["name"].get<std::string>()}, url_{""},
          instrument_type_{instrument_json["instrument_type"].get<std::string>()} {
    if (instrument_json.find("configuration") != instrument_json.end()) {
        // Configuration variables
        const auto& configuration_json = instrument_json["configuration"];
        if (configuration_json.find("unique_id") != configuration_json.end()) {
        }
        if (configuration_json.find("unique_id") != configuration_json.end()) {
            unique_id_ = configuration_json["unique_id"].get<std::string>();
        }
        if (configuration_json.find("platform") != configuration_json.end()) {
            platform_ = configuration_json["platform"].get<std::string>();
        }
        if (configuration_json.find("release_version") != configuration_json.end()) {
            release_version_ = configuration_json["release_version"].get<std::string>();
        }
        if (configuration_json.find("firmware") != configuration_json.end()) {
            firmware_ = configuration_json["firmware"].get<std::string>();
        }
        if (configuration_json.find("firmware_version") != configuration_json.end()) {
            firmware_version_ = configuration_json["firmware_version"].get<std::string>();
        }
        if (configuration_json.find("private_ip") != configuration_json.end()) {
            private_ip_ = configuration_json["private_ip"].get<std::string>();
        }
        if (configuration_json.find("public_ip") != configuration_json.end()) {
            public_ip_ = configuration_json["public_ip"].get<std::string>();
        }
        if (configuration_json.find("host") != configuration_json.end()) {
            host_ = configuration_json["host"].get<std::string>();
        }
        if (configuration_json.find("mac_address") != configuration_json.end()) {
            mac_address_ = configuration_json["mac_address"].get<std::string>();
        }
        if (configuration_json.find("timezone") != configuration_json.end()) {
            timezone_ = configuration_json["timezone"].get<std::string>();
        }
        if (configuration_json.find("physical_address") != configuration_json.end()) {
            const auto& physical_address_json = configuration_json["physical_address"];
            if (physical_address_json.find("country_code") != physical_address_json.end()) {
                physical_address_.country_code =
                        physical_address_json["country_code"].get<std::string>();
                physical_address_.is_set = true;
            }
            if (physical_address_json.find("country_name") != physical_address_json.end()) {
                physical_address_.country_name =
                        physical_address_json["country_name"].get<std::string>();
                physical_address_.is_set = true;
            }
            if (physical_address_json.find("city") != physical_address_json.end()) {
                physical_address_.city =
                        physical_address_json["city"].get<std::string>();
                physical_address_.is_set = true;
            }
            if (physical_address_json.find("zip_code") != physical_address_json.end()) {
                physical_address_.zip_code =
                        physical_address_json["zip_code"].get<std::string>();
                physical_address_.is_set = true;
            }
        }
        if (configuration_json.find("geo_location") != configuration_json.end()) {
            const auto& geo_location_json = configuration_json["geo_location"];
            if (geo_location_json.find("latitude") != geo_location_json.end()) {
                geo_location_.latitude = geo_location_json["latitude"].get<double>();
                geo_location_.is_set = true;
            }
            if (geo_location_json.find("longitude") != geo_location_json.end()) {
                geo_location_.longitude = geo_location_json["longitude"].get<double>();
                geo_location_.is_set = true;
            }
        }
    }

    if (instrument_json.find("metadata") != instrument_json.end()) {
        // Metadata variables
        const auto& metadata_json = instrument_json["metadata"];
        if (metadata_json.find("manufacturer") != metadata_json.end()) {
            manufacturer_ = metadata_json["manufacturer"].get<std::string>();
        }
        if (metadata_json.find("model") != metadata_json.end()) {
            model_ = metadata_json["model"].get<std::string>();
        }
        if (metadata_json.find("width") != metadata_json.end()) {
            width_ = metadata_json["width"].get<std::uint32_t>();
        }
        if (metadata_json.find("height") != metadata_json.end()) {
            height_ = metadata_json["height"].get<std::uint32_t>();
        }
        if (metadata_json.find("framerate") != metadata_json.end()) {
            framerate_ = metadata_json["framerate"].get<std::uint32_t>();
        }
        if (metadata_json.find("cpu_info") != metadata_json.end()) {
            cpu_info_ = metadata_json["cpu_info"];
        }
        if (metadata_json.find("disk_info") != metadata_json.end()) {
            disk_info_ = metadata_json["disk_info"];
        }
        if (metadata_json.find("memory_info") != metadata_json.end()) {
            memory_info_ = metadata_json["memory_info"];
        }
    }
}

nlohmann::json Instrument::ToJson() const {
    nlohmann::json json;

    if (name_.empty() && instrument_type_.empty()) {
        return json;
    }

    json["name"] = name_;
    json["url"] = url_;
    json["instrument_type"] = instrument_type_;

    nlohmann::json configuration;

    if (!unique_id_.empty()) {
        configuration["unique_id"] = unique_id_;
    }
    if (!platform_.empty()) {
        configuration["platform"] = platform_;
    }
    if (!release_version_.empty()) {
        configuration["release_version"] = release_version_;
    }
    if (!firmware_.empty()) {
        configuration["firmware"] = firmware_;
    }
    if (!firmware_version_.empty()) {
        configuration["firmware_version"] = firmware_version_;
    }
    if (!private_ip_.empty()) {
        configuration["private_ip"] = private_ip_;
    }
    if (!public_ip_.empty()) {
        configuration["public_ip"] = public_ip_;
    }
    if (!host_.empty()) {
        configuration["host"] = host_;
    }
    if (!mac_address_.empty()) {
        configuration["mac_address"] = mac_address_;
    }
    if (!timezone_.empty()) {
        configuration["timezone"] = timezone_;
    }
    if (physical_address_.is_set) {
        nlohmann::json physical;
        if (!physical_address_.country_code.empty()) {
            physical["country_code"] = physical_address_.country_code;
        }
        if (!physical_address_.country_name.empty()) {
            physical["country_name"] = physical_address_.country_name;
        }
        if (!physical_address_.city.empty()) {
            physical["city"] = physical_address_.city;
        }
        if (!physical_address_.zip_code.empty()) {
            physical["zip_code"] = physical_address_.zip_code;
        }
        configuration["physical_address"] = physical;
    }
    if (geo_location_.is_set) {
        nlohmann::json geo;
        geo["latitude"] = geo_location_.latitude;
        geo["longitude"] = geo_location_.longitude;
        configuration["geo_location"] = geo;
    }

    if (!configuration.empty()) {
        json["configuration"] = configuration;
    }

    nlohmann::json metadata;

    if (!manufacturer_.empty()) {
        metadata["manufacturer"] = manufacturer_;
    }
    if (!model_.empty()) {
        metadata["model"] = model_;
    }
    if (width_ > 0) {
        metadata["width"] = width_;
    }
    if (height_ > 0) {
        metadata["height"] = height_;
    }
    if (framerate_ > 0) {
        metadata["framerate"] = framerate_;
    }
    if (!cpu_info_.empty()) {
        metadata["cpu_info"] = cpu_info_;
    }
    if (!disk_info_.empty()) {
        metadata["disk_info"] = disk_info_;
    }
    if (!memory_info_.empty()) {
        metadata["memory_info"] = memory_info_;
    }

    if (!metadata.empty()) {
        json["metadata"] = metadata;
    }

    return json;
}

Instrument::operator bool() const {
    return id_ > 0;
}

} // namespace api
} // namespace connect
} // namespace prism
