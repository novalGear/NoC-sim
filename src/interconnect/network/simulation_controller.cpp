/**
 * @file simulation_controller.cpp
 * @brief Реализация класса SimulationController.
 * @author Novoselov Alexander
 * @date 14/04/2026
 */

#include "simulation_controller.hpp"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void SimulationController::setup_mesh(int width, int height) {
    interconnect_ = std::make_unique<MeshInterconnect>(width, height);
}

void SimulationController::build() {
    if (!interconnect_) {
        throw std::runtime_error("Topology not set up. Call setup_mesh() first.");
    }
    interconnect_->build();
    is_built_ = true;
}

void SimulationController::create_nodes() {
    if (!interconnect_) return;

    nodes_.clear();
    int num_nodes = interconnect_->get_total_nodes();
    for (int i = 0; i < num_nodes; ++i) {
        nodes_.push_back(std::make_unique<NetworkNode>(i, *interconnect_, trace_));
    }
}

//=============================================================================
// Трейсы
//=============================================================================

void SimulationController::generate_uniform_trace(int total_packets, double injection_rate, int max_ticks) {
    if (!interconnect_) {
        throw std::runtime_error("Topology not set up.");
    }
    int num_nodes = interconnect_->get_total_nodes();
    trace_.generate_uniform(total_packets, num_nodes, injection_rate, max_ticks);
    create_nodes();
}

void SimulationController::generate_hotspot_trace(int total_packets, int hotspot_node,
                                                  double hotspot_ratio, double injection_rate, int max_ticks) {
    if (!interconnect_) {
        throw std::runtime_error("Topology not set up.");
    }
    int num_nodes = interconnect_->get_total_nodes();
    trace_.generate_hotspot(total_packets, num_nodes, hotspot_node,
                            hotspot_ratio, injection_rate, max_ticks);
    create_nodes();
}

void SimulationController::generate_synthetic_trace(const std::string& pattern) {
    if (!interconnect_) {
        throw std::runtime_error("Topology not set up.");
    }
    int num_nodes = interconnect_->get_total_nodes();
    int width = interconnect_->get_width();
    int height = interconnect_->get_height();
    trace_.generate_synthetic(pattern, num_nodes, width, height);
    create_nodes();
}

void SimulationController::load_trace(const std::string& filename) {
    trace_.load_from_json(filename);
    create_nodes();
}

void SimulationController::save_trace(const std::string& filename) const {
    trace_.save_to_json(filename);
}

//=============================================================================
// Запуск
//=============================================================================

void SimulationController::run(int max_ticks) {
    if (!is_built_) {
        build();
    }

    std::cout << "Starting simulation for " << max_ticks << " ticks..." << std::endl;
    std::cout << "Total packets: " << trace_.total_packets() << std::endl;

    for (current_tick_ = 0; current_tick_ < max_ticks; ++current_tick_) {
        // Узлы отправляют/принимают
        for (auto& node : nodes_) {
            node->on_clock(current_tick_);
        }

        // Интерконнект делает такт
        interconnect_->on_clock();

        // Прогресс
        if (max_ticks > 0 && current_tick_ % (max_ticks / 10) == 0 && current_tick_ > 0) {
            std::cout << "  Progress: " << (current_tick_ * 100 / max_ticks) << "%" << std::endl;
        }
    }

    std::cout << "Simulation finished." << std::endl;
    dump_results();
}

void SimulationController::step() {
    if (!is_built_) {
        build();
    }

    for (auto& node : nodes_) {
        node->on_clock(current_tick_);
    }
    interconnect_->on_clock();
    current_tick_++;
}

//=============================================================================
// Результаты
//=============================================================================

void SimulationController::dump_results() const {
    std::cout << "\n=== Simulation Results ===" << std::endl;
    trace_.print_stats();

    std::cout << "\n=== Per-Node Statistics ===" << std::endl;
    for (const auto& node : nodes_) {
        node->print_stats();
    }
}

void SimulationController::save_results(const std::string& filename) const {
    json results;

    // Общая статистика
    results["total_packets"]    = trace_.total_packets();
    results["delivered"]        = trace_.delivered_count();
    results["lost"]             = trace_.lost_count();
    results["average_latency"]  = trace_.average_latency();
    results["average_hops"]     = trace_.average_hops();

    // Статистика по узлам
    json nodes_json = json::array();
    for (const auto& node : nodes_) {
        nodes_json.push_back({
            {"node_id",         node->get_id()},
            {"sent",            node->packets_sent()},
            {"received",        node->packets_received()},
            {"send_failures",   node->send_failures()}
        });
    }
    results["nodes"] = nodes_json;

    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot write to file: " + filename);
    }
    file << results.dump(4);

    std::cout << "Results saved to " << filename << std::endl;
}
