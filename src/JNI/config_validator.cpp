#include "config_validator.hpp"
#include "logger.hpp"
#include <vector>

namespace config_validator {

ValidationResult check_required_fields(const json& config) {
    std::vector<std::string> required = {
        "topology", "routing", "traffic", "width", "height",
        "nodes", "max_ticks", "total_packets", "injection_rate"
    };

    for (const auto& field : required) {
        if (!config.contains(field)) {
            return {false, "Missing required field: " + field};
        }
    }

    return {true, ""};
}

ValidationResult check_topology_routing(const json& config) {
    std::string topology = config["topology"];
    std::string routing = config["routing"];
    std::string traffic = config["traffic"];

    if (topology != "MESH") {
        return {false, "Only MESH topology is supported"};
    }

    if (routing != "STATIC") {
        return {false, "Only STATIC (X-Y) routing is supported"};
    }

    if (traffic != "UNIFORM" && traffic != "HOTSPOT") {
        return {false, "Only UNIFORM and HOTSPOT traffic patterns are supported"};
    }

    return {true, ""};
}

ValidationResult check_numeric_params(const json& config) {
    try {
        int width = config["width"];
        int height = config["height"];
        int nodes = config["nodes"];
        int max_ticks = config["max_ticks"];
        int total_packets = config["total_packets"];
        double injection_rate = config["injection_rate"];

        if (width <= 0 || width > 64) {
            return {false, "Width must be between 1 and 64"};
        }
        if (height <= 0 || height > 64) {
            return {false, "Height must be between 1 and 64"};
        }
        if (nodes != width * height) {
            return {false, "For MESH topology, nodes must equal width * height"};
        }
        if (max_ticks <= 0 || max_ticks > 1000000) {
            return {false, "max_ticks must be between 1 and 1,000,000"};
        }
        if (total_packets <= 0 || total_packets > 100000) {
            return {false, "total_packets must be between 1 and 100,000"};
        }
        if (injection_rate <= 0.0 || injection_rate > 1.0) {
            return {false, "injection_rate must be between 0.0 and 1.0"};
        }
    } catch (const std::exception& e) {
        return {false, std::string("Invalid parameter type: ") + e.what()};
    }

    return {true, ""};
}

ValidationResult validate_all(const json& config) {
    auto field_check = check_required_fields(config);
    if (!field_check.valid) return field_check;

    auto topo_check = check_topology_routing(config);
    if (!topo_check.valid) return topo_check;

    return check_numeric_params(config);
}

void log_configuration(const json& config) {
    jni_logger::info("Configuration loaded successfully:");
    jni_logger::info("  Topology: " + config["topology"].get<std::string>());
    jni_logger::info("  Routing: " + config["routing"].get<std::string>());
    jni_logger::info("  Traffic: " + config["traffic"].get<std::string>());
    jni_logger::info("  Size: " + std::to_string(config["width"].get<int>()) +
                     "x" + std::to_string(config["height"].get<int>()));
    jni_logger::info("  Total packets: " + std::to_string(config["total_packets"].get<int>()));
    jni_logger::info("  Max ticks: " + std::to_string(config["max_ticks"].get<int>()));
    jni_logger::info("  Injection rate: " + std::to_string(config["injection_rate"].get<double>()));
}

} // namespace config_validator
