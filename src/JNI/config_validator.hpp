#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace config_validator {

using json = nlohmann::json;

struct ValidationResult {
    bool valid;
    std::string error_message;
};

// Проверка наличия всех обязательных полей
ValidationResult check_required_fields(const json& config);

// Проверка топологии и роутинга
ValidationResult check_topology_routing(const json& config);

// Проверка числовых параметров
ValidationResult check_numeric_params(const json& config);

// Полная валидация конфигурации
ValidationResult validate_all(const json& config);

// Вывод конфигурации в лог
void log_configuration(const json& config);

} // namespace config_validator
