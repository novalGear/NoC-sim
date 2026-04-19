/**
 * @file packet_trace.cpp
 * @brief Реализация класса PacketTrace.
 * @author Novoselov Alexander
 * @date 14/04/2026
 */

#include "packet_trace.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

//=============================================================================
// Конструкторы
//=============================================================================

PacketTrace::PacketTrace() : rng_(std::random_device{}()) {}

PacketTrace::PacketTrace(unsigned seed) : rng_(seed) {}

//=============================================================================
// Приватные методы
//=============================================================================

void PacketTrace::build_send_tick_index() {
    if (packets_.empty()) return;

    max_tick_ = 0;
    for (const auto& pkt : packets_) {
        if (pkt.send_tick > max_tick_) {
            max_tick_ = pkt.send_tick;
        }
    }

    packets_by_send_tick_.clear();
    packets_by_send_tick_.resize(max_tick_ + 1);

    for (size_t i = 0; i < packets_.size(); ++i) {
        int tick = packets_[i].send_tick;
        if (tick >= 0 && tick <= max_tick_) {
            packets_by_send_tick_[tick].push_back(static_cast<int>(i));
        }
    }
}

void PacketTrace::distribute_send_ticks(double injection_rate, int max_ticks) {
    std::exponential_distribution<> exp_dist(injection_rate);

    for (auto& pkt : packets_) {
        double sample;
        do {
            sample = exp_dist(rng_);
        } while (sample >= max_ticks);
        pkt.send_tick = static_cast<int>(sample);
    }

    std::sort(packets_.begin(), packets_.end(),
              [](const Packet& a, const Packet& b) {
                  return a.send_tick < b.send_tick;
              });

    for (size_t i = 0; i < packets_.size(); ++i) {
        packets_[i].id = static_cast<int>(i);
    }
    next_id_ = static_cast<int>(packets_.size());

    build_send_tick_index();
}

//=============================================================================
// Генерация трейсов
//=============================================================================

void PacketTrace::generate_uniform(int total_packets, int num_nodes,
                                   double injection_rate, int max_ticks) {
    packets_.clear();
    injected_.clear();
    delivered_.clear();
    next_id_ = 0;

    std::uniform_int_distribution<> node_dist(0, num_nodes - 1);

    for (int i = 0; i < total_packets; ++i) {
        int src = node_dist(rng_);
        int dst = node_dist(rng_);
        while (dst == src) {
            dst = node_dist(rng_);
        }
        packets_.emplace_back(i, src, dst, -1, 1);
    }

    distribute_send_ticks(injection_rate, max_ticks);

    injected_.resize(packets_.size(), false);
    delivered_.resize(packets_.size(), false);
}

void PacketTrace::generate_hotspot(int total_packets, int num_nodes, int hotspot_node,
                                   double hotspot_ratio, double injection_rate, int max_ticks) {
    packets_.clear();
    injected_.clear();
    delivered_.clear();
    next_id_ = 0;

    std::uniform_int_distribution<> node_dist(0, num_nodes - 1);
    std::uniform_real_distribution<> type_dist(0.0, 1.0);

    for (int i = 0; i < total_packets; ++i) {
        int src = node_dist(rng_);
        int dst;

        if (type_dist(rng_) < hotspot_ratio) {
            dst = hotspot_node;
            if (dst == src) {
                dst = (dst + 1) % num_nodes;
            }
        } else {
            dst = node_dist(rng_);
            while (dst == src) {
                dst = node_dist(rng_);
            }
        }

        packets_.emplace_back(i, src, dst, -1, 1);
    }

    distribute_send_ticks(injection_rate, max_ticks);

    injected_.resize(packets_.size(), false);
    delivered_.resize(packets_.size(), false);
}

void PacketTrace::generate_synthetic(const std::string& pattern, int num_nodes,
                                     int width, int height) {
    packets_.clear();
    injected_.clear();
    delivered_.clear();
    next_id_ = 0;

    if (pattern == "transpose" && width > 0 && height > 0) {
        for (int src = 0; src < num_nodes; ++src) {
            int src_x = src % width;
            int src_y = src / width;
            int dst_x = src_y;
            int dst_y = src_x;
            if (dst_x < width && dst_y < height) {
                int dst = dst_y * width + dst_x;
                if (src != dst) {
                    packets_.emplace_back(next_id_++, src, dst, 0, 1);
                }
            }
        }
    }
    else if (pattern == "bit_reversal") {
        int log2 = 0;
        while ((1 << log2) < num_nodes) log2++;

        for (int src = 0; src < num_nodes; ++src) {
            int dst = 0;
            for (int b = 0; b < log2; ++b) {
                if (src & (1 << b)) {
                    dst |= (1 << (log2 - 1 - b));
                }
            }
            if (src != dst && dst < num_nodes) {
                packets_.emplace_back(next_id_++, src, dst, 0, 1);
            }
        }
    }
    else if (pattern == "shuffle") {
        int half = num_nodes / 2;
        for (int src = 0; src < num_nodes; ++src) {
            int dst = (src * 2) % num_nodes;
            if (src < half) dst++;
            if (src != dst) {
                packets_.emplace_back(next_id_++, src, dst, 0, 1);
            }
        }
    }

    build_send_tick_index();

    injected_.resize(packets_.size(), false);
    delivered_.resize(packets_.size(), false);
}

//=============================================================================
// Загрузка/сохранение
//=============================================================================

void PacketTrace::load_from_json(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    json data;
    file >> data;

    packets_.clear();
    next_id_ = 0;
    max_tick_ = 0;

    for (const auto& item : data["packets"]) {
        Packet pkt;
        pkt.id = item.value("id", next_id_);
        pkt.src = item["src"];
        pkt.dst = item["dst"];
        pkt.send_tick = item.value("send_tick", -1);
        pkt.recv_tick = item.value("recv_tick", -1);
        pkt.hops = item.value("hops", 0);
        pkt.size = item.value("size", 1);

        packets_.push_back(pkt);
        next_id_ = std::max(next_id_, pkt.id + 1);
        if (pkt.send_tick > max_tick_) {
            max_tick_ = pkt.send_tick;
        }
    }

    build_send_tick_index();

    injected_.resize(packets_.size(), false);
    delivered_.resize(packets_.size(), false);
}

void PacketTrace::save_to_json(const std::string& filename) const {
    json data;
    data["metadata"]["total_packets"] = packets_.size();
    data["metadata"]["max_tick"] = max_tick_;

    json packets_json = json::array();
    for (const auto& pkt : packets_) {
        packets_json.push_back({
            {"id", pkt.id},
            {"src", pkt.src},
            {"dst", pkt.dst},
            {"send_tick", pkt.send_tick},
            {"recv_tick", pkt.recv_tick},
            {"hops", pkt.hops},
            {"size", pkt.size}
        });
    }
    data["packets"] = packets_json;

    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot write to file: " + filename);
    }
    file << data.dump(4);
}

//=============================================================================
// Доступ к данным
//=============================================================================

std::vector<int> PacketTrace::get_packets_to_send(int tick) const {
    if (tick < 0 || tick >= static_cast<int>(packets_by_send_tick_.size())) {
        return {};
    }
    return packets_by_send_tick_[tick];
}

int PacketTrace::find_packet_by_id(int id) const {
    for (size_t i = 0; i < packets_.size(); ++i) {
        if (packets_[i].id == id) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

//=============================================================================
// Обновление состояния
//=============================================================================

void PacketTrace::mark_injected(int packet_idx, int tick) {
    if (packet_idx >= 0 && packet_idx < static_cast<int>(packets_.size())) {
        packets_[packet_idx].send_tick = tick;
        injected_[packet_idx] = true;
    }
}

void PacketTrace::mark_delivered(int packet_idx, int tick, int hops) {
    if (packet_idx >= 0 && packet_idx < static_cast<int>(packets_.size())) {
        packets_[packet_idx].recv_tick = tick;
        packets_[packet_idx].hops = hops;
        delivered_[packet_idx] = true;
    }
}

void PacketTrace::increment_hops(int packet_idx) {
    if (packet_idx >= 0 && packet_idx < static_cast<int>(packets_.size())) {
        packets_[packet_idx].hops++;
    }
}

//=============================================================================
// Статистика
//=============================================================================

int PacketTrace::delivered_count() const {
    return static_cast<int>(std::count(delivered_.begin(), delivered_.end(), true));
}

int PacketTrace::lost_count() const {
    return total_packets() - delivered_count();
}

double PacketTrace::average_latency() const {
    long long sum = 0;
    int count = 0;
    for (const auto& pkt : packets_) {
        int lat = pkt.latency();
        if (lat >= 0) {
            sum += lat;
            count++;
        }
    }
    return count > 0 ? static_cast<double>(sum) / count : 0.0;
}

double PacketTrace::average_hops() const {
    long long sum = 0;
    int count = 0;
    for (const auto& pkt : packets_) {
        if (pkt.hops > 0) {
            sum += pkt.hops;
            count++;
        }
    }
    return count > 0 ? static_cast<double>(sum) / count : 0.0;
}

void PacketTrace::print_stats() const {
    std::cout << "=== PacketTrace Statistics ===" << std::endl;
    std::cout << "Total packets: " << total_packets() << std::endl;
    std::cout << "Delivered: " << delivered_count() << std::endl;
    std::cout << "Lost: " << lost_count() << std::endl;
    std::cout << "Average latency: " << average_latency() << " ticks" << std::endl;
    std::cout << "Average hops: " << average_hops() << std::endl;
    std::cout << "Max tick: " << max_tick() << std::endl;
}
