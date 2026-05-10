/**
@file network_node.tpp
@brief Implementation of NetworkNode.
*/
#pragma once
#include "network_node.hpp"
#include <iostream>
#include "debug.hpp"

template <typename ArbiterT>
NetworkNode<ArbiterT>::NetworkNode(int id, MeshInterconnect<ArbiterT>& interconnect, PacketTrace& trace)
    : node_id_(id), interconnect_(interconnect), trace_(trace) {
    collect_my_packets();
}

template <typename ArbiterT>
void NetworkNode<ArbiterT>::collect_my_packets() {
    for (int i = 0; i < trace_.total_packets(); ++i) {
        if (trace_.packets()[i].src == node_id_) {
            my_packet_indices_.push_back(i);
        }
    }
}

template <typename ArbiterT>
void NetworkNode<ArbiterT>::on_clock(int current_tick) {
    // Отправка
    while (next_packet_idx_ < my_packet_indices_.size()) {
        int pkt_idx = my_packet_indices_[next_packet_idx_];
        const Packet& pkt = trace_.packets()[pkt_idx];

        if (pkt.send_tick > current_tick) break;

        if (interconnect_.inject_packet(node_id_, pkt)) {
            trace_.mark_injected(pkt_idx, current_tick);
            stats_.packets_sent++;
            next_packet_idx_++;
            DEBUG_PRINT("Node " << node_id_ << ": injected packet " << pkt.id
                        << " with hops=" << pkt.hops);
        } else {
            stats_.send_failures++;
            break;
        }
    }

    // Прием
    while (auto pkt = interconnect_.eject_packet(node_id_)) {
        int pkt_idx = trace_.find_packet_by_id(pkt->id);
        if (pkt_idx != -1) {
            // ОТЛАДКА: выводим hops при получении
            DEBUG_PRINT("Node " << node_id_ << ": received packet " << pkt->id
                        << " with hops=" << pkt->hops);
            trace_.mark_delivered(pkt_idx, current_tick, pkt->hops);
            stats_.packets_received++;
        }
        recv_queue_.push(*pkt);
    }
}

template <typename ArbiterT>
bool NetworkNode<ArbiterT>::has_pending_packets() const {
    return next_packet_idx_ < my_packet_indices_.size();
}

template <typename ArbiterT>
void NetworkNode<ArbiterT>::print_stats() const {
    std::cout << "Node " << node_id_ << ": " << std::endl;
    std::cout << "  Sent: " << stats_.packets_sent << std::endl;
    std::cout << "  Received: " << stats_.packets_received << std::endl;
    std::cout << "  Send failures: " << stats_.send_failures << std::endl;
}
