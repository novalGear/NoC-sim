#pragma once
#include <nlohmann/json.hpp>
#include "simulation_controller.hpp"
#include "rr_arbiter.hpp"

namespace simulation_runner {
    using json = nlohmann::json;

    // Шаблонные внутренние функции для работы с конкретным типом арбитра
    template <typename ArbiterT>
    SimulationController<ArbiterT> create_and_setup_controller(const json& config);

    template <typename ArbiterT>
    void generate_traffic(SimulationController<ArbiterT>& controller, const json& config);

    template <typename ArbiterT>
    json collect_results(const SimulationController<ArbiterT>& controller,
                         const json& config,
                         long long execution_time_ms);

    // Публичный API для JNI (диспетчеризация внутри .cpp)
    json run_simulation(const json& config);
}
