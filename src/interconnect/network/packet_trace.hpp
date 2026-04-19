/**
 * @file packet_trace.hpp
 * @brief Класс для хранения и генерации трейсов пакетов.
 * @author Novoselov Alexander
 * @date 14/04/2026
 */

#pragma once

#include "packet.hpp"
#include <vector>
#include <string>
#include <random>

/**
 * @class PacketTrace
 * @brief Хранит все пакеты для симуляции и предоставляет методы генерации/загрузки.
 */
class PacketTrace {
private:
    std::vector<Packet> packets_;
    std::vector<bool> injected_;
    std::vector<bool> delivered_;
    std::vector<std::vector<int>> packets_by_send_tick_;

    int next_id_ = 0;
    int max_tick_ = 0;
    std::mt19937 rng_;

    void build_send_tick_index();
    void distribute_send_ticks(double injection_rate, int max_ticks);

public:
    PacketTrace();
    explicit PacketTrace(unsigned seed);

    // Генерация трейсов
    void generate_uniform(int total_packets, int num_nodes,
                          double injection_rate, int max_ticks);
    void generate_hotspot(int total_packets, int num_nodes, int hotspot_node,
                          double hotspot_ratio, double injection_rate, int max_ticks);
    void generate_synthetic(const std::string& pattern, int num_nodes,
                            int width = 0, int height = 0);

    // Загрузка/сохранение
    void load_from_json(const std::string& filename);
    void save_to_json(const std::string& filename) const;

    // Доступ к данным
    const std::vector<Packet>& packets() const { return packets_; }
    const std::vector<bool>& injected() const { return injected_; }
    const std::vector<bool>& delivered() const { return delivered_; }
    int total_packets() const { return static_cast<int>(packets_.size()); }
    int max_tick() const { return max_tick_; }

    std::vector<int> get_packets_to_send(int tick) const;
    int find_packet_by_id(int id) const;

    // Обновление состояния
    void mark_injected(int packet_idx, int tick);
    void mark_delivered(int packet_idx, int tick);
    void increment_hops(int packet_idx);

    // Статистика
    int delivered_count() const;
    int lost_count() const;
    double average_latency() const;
    double average_hops() const;
    void print_stats() const;
};
