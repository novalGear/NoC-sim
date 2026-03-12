#pragma once

#include "packet.h"
#include "port.h"

#include <cassert>
#include <vector>

class Router {
private:
    int id;

    std::vector<Port> input_ports;
    std::vector<Port> output_ports;
    int in_port_count;
    int out_port_count;
    Arbiter arbiter;

public:
    Router(int input_count, int output_count)
    void on_clock();

    // определить номер выходного порта для пакета
    int route_pkt(Packet& pkt);
    void route_all();
    void arbitrate_all();

};



void Router::on_clock() {
    int input_ports_count = in_port_count;
    AllRequests requests;
    std::vector<int> senders_list;
    collect_requests(requests);
    arbitrate_all(requests, senders_list);
    send_all(senders_list);
}

void Router::collect_requests(AllRequests& requests) {
    requests.resize(out_port_count);

    for (int in_port_idx = 0; in_port_idx < input_ports_count; ++in_port_idx) {
        Port& in_port = input_ports[in_port_idx];
        if (in_port.hasData()) {
            std::optional<Packet> pkt = in_port.tryRecv();
            assert(pkt.has_value());

            int dst_port = route_pkt(pkt);
            requests[dst_port].pushback(in_port);
        }
    }
}

void Router::arbitrate_all(AllRequests& requests) {
    senders_list.resize(out_port_count);
    int req_list_size = requests.size();
    for (int dst_port = 0; dst_port < req_list_size; ++dst_port) {
        RequestsList& req_list = requests[dst_port];
        int winning_input_idx = arbiter.arbitrate(req_list, dst_port);
        senders_list[dst_port] = req_list[winning_input_idx];
    }
}

void Router::send_all(std::vector<int> senders_list) {
    for (int dst_port_idx = 0; dst_port_idx < out_port_count; ++dst_port_idx) {
        Port& dst_port = output_ports[dst_port_idx];
        if (dst_port.isReady()) {
            int src_port_idx = senders_list[dst_port_idx];
            Port& src_port = input_ports[src_port_idx];
            assert(src_port.hasData());

            std::optional<Packet> pkt = src_port.tryRecv();
            assert(pkt.has_value());

            dst_port.trySend(pkt);
        } else {

        }
    }
}
