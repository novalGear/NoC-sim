/**
 * @file simulation_controller.hpp
 * @brief Контроллер симуляции NoC.
 * @author Novoselov Alexander
 * @date 14/04/2026
 */

#pragma once

#include "mesh.hpp"
#include "packet_trace.hpp"
#include "network_node.hpp"
#include <memory>
#include <vector>

class SimulationController {
private:
    std::unique_ptr<MeshInterconnect> interconnect_;
    PacketTrace trace_;
    std::vector<std::unique_ptr<NetworkNode>> nodes_;
    int current_tick_ = 0;
    bool is_built_ = false;

    void create_nodes();

public:
    SimulationController() = default;

    // Настройка топологии
    void setup_mesh(int width, int height);
    void build();

    // Трейсы
    void generate_uniform_trace(int total_packets, double injection_rate, int max_ticks);
    void generate_hotspot_trace(int total_packets, int hotspot_node,
                                double hotspot_ratio, double injection_rate, int max_ticks);
    void generate_synthetic_trace(const std::string& pattern);
    void load_trace(const std::string& filename);
    void save_trace(const std::string& filename) const;

    // Запуск
    void run(int max_ticks);
    void step();  // один такт (для отладки)

    // Результаты
    void dump_results() const;
    void save_results(const std::string& filename) const;

    // Геттеры
    MeshInterconnect* get_interconnect() { return interconnect_.get(); }
    const PacketTrace& get_trace() const { return trace_; }
};
