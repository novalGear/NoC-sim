#pragma once
#include <string>
#include <map>

// Типы топологии
enum class TopologyType { MESH, BUTTERFLY, UNKNOWN };
enum class RoutingType { OBLIVIOUS, ADAPTIVE, UNKNOWN };

// Конфигурация интерконнекта (входные данные)
struct InterconnectConfig {
    int width = 0;
    int height = 0;
    int nodes = 0;
    TopologyType topology = TopologyType::UNKNOWN;
    RoutingType routing = RoutingType::UNKNOWN;

    bool isValid() const {
        return width > 0 && height > 0 && nodes > 0 &&
               topology != TopologyType::UNKNOWN &&
               routing != RoutingType::UNKNOWN;
    }
};

// Результаты симуляции (выходные данные)
struct SimulationResult {
    bool success = false;
    double latency_avg = 0.0;      // средняя задержка
    double latency_max = 0.0;      // максимальная задержка
    double throughput = 0.0;       // пропускная способность (пакетов/такт)
    int packets_sent = 0;
    int packets_delivered = 0;
    int packets_lost = 0;
    long long cycles_elapsed = 0;  // тактов симуляции
    long long execution_time_ms = 0;
    std::string errorMessage;

    // Конвертация результата в JSON-строку (без библиотек)
    std::string toJson() const {
        char buffer[1024];
        snprintf(buffer, sizeof(buffer),
            "{\"success\":%s,\"latency_avg\":%.3f,\"latency_max\":%.3f,"
            "\"throughput\":%.2f,\"packets_sent\":%d,\"packets_delivered\":%d,"
            "\"packets_lost\":%d,\"cycles_elapsed\":%lld,\"error\":\"%s\"}",
            success ? "true" : "false",
            latency_avg, latency_max, throughput,
            packets_sent, packets_delivered, packets_lost,
            cycles_elapsed, errorMessage.c_str());
        return std::string(buffer);
    }
};
