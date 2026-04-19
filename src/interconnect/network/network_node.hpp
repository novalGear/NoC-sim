/**
 * @file network_node.hpp
 * @brief IP Core, подключенный к роутеру через LOCAL порт.
 * @author Novoselov Alexander
 * @date 14/04/2026
 */

#pragma once

#include "packet_trace.hpp"
#include "mesh.hpp"
#include <queue>
#include <optional>

class NetworkNode {
private:
    int node_id_;
    MeshInterconnect& interconnect_;
    PacketTrace& trace_;

    std::vector<int> my_packet_indices_;
    size_t next_packet_idx_ = 0;
    std::queue<Packet> recv_queue_;

    struct {
        int packets_sent = 0;
        int packets_received = 0;
        int send_failures = 0;
    } stats_;

    void collect_my_packets();

public:
    NetworkNode(int id, MeshInterconnect& interconnect, PacketTrace& trace);

    void on_clock(int current_tick);
    bool has_pending_packets() const;

    int get_id() const { return node_id_; }
    int packets_sent() const { return stats_.packets_sent; }
    int packets_received() const { return stats_.packets_received; }
    int send_failures() const { return stats_.send_failures; }

    void print_stats() const;
};
