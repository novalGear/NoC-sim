#include "simulation_runner.hpp"
#include "logger.hpp"
#include "json_utils.hpp"
#include <chrono>

namespace simulation_runner {

// -----------------------------------------------------------------------------
// Шаблонная реализация логики (сохраняем модульность)
// -----------------------------------------------------------------------------

template <typename ArbiterT>
SimulationController<ArbiterT> create_and_setup_controller(const json& config) {
    int width = json_utils::get_int(config, "width", 8);
    int height = json_utils::get_int(config, "height", 8);

    jni_logger::info("Creating simulation controller...");

    SimulationController<ArbiterT> controller;
    controller.setup_mesh(width, height);
    controller.build();

    jni_logger::info("Mesh " + std::to_string(width) + "x" +
                     std::to_string(height) + " built");

    return controller;
}

template <typename ArbiterT>
void generate_traffic(SimulationController<ArbiterT>& controller, const json& config) {
    std::string traffic   = json_utils::get_string(config, "traffic", "UNIFORM");
    int total_packets     = json_utils::get_int(config,    "total_packets", 1000);
    double injection_rate = json_utils::get_double(config, "injection_rate", 0.1);
    int max_ticks         = json_utils::get_int(config,    "max_ticks", 10000);

    if (traffic == "UNIFORM") {
        jni_logger::info("Generating UNIFORM traffic: " +
                         std::to_string(total_packets) + " packets, rate=" +
                         std::to_string(injection_rate));
        controller.generate_uniform_trace(total_packets, injection_rate, max_ticks);
    } else if (traffic == "HOTSPOT") {
        int hotspot_node = 0; // Можно вынести в конфиг при необходимости
        double hotspot_ratio = 0.5;

        jni_logger::info("Generating HOTSPOT traffic: " +
                         std::to_string(total_packets) + " packets, hotspot=" +
                         std::to_string(hotspot_node));

        controller.generate_hotspot_trace(total_packets, hotspot_node,
                                          hotspot_ratio, injection_rate, max_ticks);
    }
}

template <typename ArbiterT>
json collect_results(const SimulationController<ArbiterT>& controller,
                     const json& config,
                     long long execution_time_ms) {
    const PacketTrace& trace = controller.get_trace();

    json result = json_utils::make_success_response();

    // Основная статистика
    result["total_packets"]      = trace.total_packets();
    result["packets_sent"]       = trace.total_packets();
    result["packets_delivered"]  = trace.delivered_count();
    result["packets_lost"]       = trace.lost_count();
    result["latency_avg"]        = trace.average_latency();
    result["latency_max"]        = 0.0; // TODO: реализовать в PacketTrace
    result["average_hops"]       = trace.average_hops();

    // Производные метрики
    int total = trace.total_packets();
    int delivered = trace.delivered_count();

    result["delivery_rate"] = (total > 0) ?
                              static_cast<double>(delivered) / total : 0.0;

    int max_ticks = json_utils::get_int(config, "max_ticks", 1);
    result["throughput"] = (max_ticks > 0) ?
                           static_cast<double>(delivered) / max_ticks : 0.0;

    result["cycles_elapsed"]    = max_ticks;
    result["execution_time_ms"] = execution_time_ms;

    return result;
}

// -----------------------------------------------------------------------------
// Явная инстанциация шаблонов для поддерживаемых типов арбитров
// -----------------------------------------------------------------------------
template SimulationController<RRArbiter> create_and_setup_controller<RRArbiter>(const json&);
template void generate_traffic<RRArbiter>(SimulationController<RRArbiter>&, const json&);
template json collect_results<RRArbiter>(const SimulationController<RRArbiter>&, const json&, long long);

// Когда добавите новые арбитры, раскомментируйте и добавьте их классы:
// template SimulationController<PriorityArbiter> create_and_setup_controller<PriorityArbiter>(const json&);
// ...

// -----------------------------------------------------------------------------
// Точка входа: Диспетчеризация по конфигу
// -----------------------------------------------------------------------------

json run_simulation(const json& config) {
    try {
        // Читаем тип арбитра из конфига (по умолчанию RR)
        std::string arb_type = json_utils::get_string(config, "arbiter_type", "RR");

        jni_logger::info("Starting simulation with arbiter: " + arb_type);

        if (arb_type == "RR") {
            // 1. Setup
            auto controller = create_and_setup_controller<RRArbiter>(config);

            // 2. Traffic
            generate_traffic(controller, config);

            // 3. Run
            int max_ticks = json_utils::get_int(config, "max_ticks", 10000);
            jni_logger::info("Running simulation for " + std::to_string(max_ticks) + " ticks...");

            auto start = std::chrono::high_resolution_clock::now();
            controller.run(max_ticks);
            auto end = std::chrono::high_resolution_clock::now();

            long long exec_time = std::chrono::duration_cast<std::chrono::milliseconds>
                                  (end - start).count();

            jni_logger::info("Simulation completed in " + std::to_string(exec_time) + " ms");

            // 4. Results
            return collect_results(controller, config, exec_time);

        } else {
            // Fallback или ошибка, если тип не поддерживается
            jni_logger::warn("Unknown arbiter type: " + arb_type + ". Fallback to RR.");
            // Рекурсивный вызов с дефолтом или возврат ошибки
            return run_simulation(json(config.begin(), config.end()));
        }

    } catch (const std::exception& e) {
        jni_logger::error(std::string("Simulation error: ") + e.what());
        return json_utils::make_error_response(e.what());
    }
}

} // namespace simulation_runner
