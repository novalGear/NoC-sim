#pragma once

#include <nlohmann/json.hpp>
#include "simulation_controller.hpp"

namespace simulation_runner {

using json = nlohmann::json;

// Запуск симуляции на основе конфигурации
json run_simulation(const json& config);

// Создание контроллера и настройка топологии
SimulationController create_and_setup_controller(const json& config);

// Генерация трафика
void generate_traffic(SimulationController& controller, const json& config);

// Сбор результатов симуляции
json collect_results(SimulationController& controller,
                     const json& config,
                     long long execution_time_ms);

} // namespace simulation_runner
