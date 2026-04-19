/**
 * @file network_node.cpp
 * @brief Реализация класса NetworkNode.
 * @author Novoselov Alexander
 * @date 14/04/2026
 */

#include "network_node.hpp"
#include <iostream>

NetworkNode::NetworkNode(int id, MeshInterconnect& interconnect, PacketTrace& trace)
    : node_id_(id), interconnect_(interconnect), trace_(trace) {
    collect_my_packets();
}

void NetworkNode::collect_my_packets() {
    for (int i = 0; i < trace_.total_packets(); ++i) {
        if (trace_.packets()[i].src == node_id_) {
            my_packet_indices_.push_back(i);
        }
    }
}

void NetworkNode::on_clock(int current_tick) {
    // Отправка
    while (next_packet_idx_ < my_packet_indices_.size()) {
        int pkt_idx = my_packet_indices_[next_packet_idx_];
        const Packet& pkt = trace_.packets()[pkt_idx];

        if (pkt.send_tick > current_tick) break;

        if (interconnect_.inject_packet(node_id_, pkt)) {
            trace_.mark_injected(pkt_idx, current_tick);
            stats_.packets_sent++;
            next_packet_idx_++;
        } else {
            stats_.send_failures++;
            break;
        }
    }

    // Прием
    while (auto pkt = interconnect_.eject_packet(node_id_)) {
        int pkt_idx = trace_.find_packet_by_id(pkt->id);
        if (pkt_idx != -1) {
            trace_.mark_delivered(pkt_idx, current_tick);
            stats_.packets_received++;
        }
        recv_queue_.push(*pkt);
    }
}

bool NetworkNode::has_pending_packets() const {
    return next_packet_idx_ < my_packet_indices_.size();
}

void NetworkNode::print_stats() const {
    std::cout << "Node " << node_id_ << ":" << std::endl;
    std::cout << "  Sent: " << stats_.packets_sent << std::endl;
    std::cout << "  Received: " << stats_.packets_received << std::endl;
    std::cout << "  Send failures: " << stats_.send_failures << std::endl;
}
