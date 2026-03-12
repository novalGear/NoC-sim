#pragma once

#include "packet.h"
#include "port.h"

class Arbiter {
private:
    int head_port_idx   = 0;
    int src_ports_count = 0;

    int calc_port_distance(int port_idx);
public:
    Arbiter(int ports_count) _src_ports_count(ports_count) {}

    int arbitrate(const std::vector<Request>& requests, int outputPortId);
};

int Arbiter::calc_port_distance(int port_idx) {
    return (port_idx + src_ports_count - head_port_idx) % src_ports_count;
}

int Arbiter::arbitrate(const std::vector<Request>& requests, int outputPortId); {
    int res_req_idx = 0;
    int least_dist = src_ports_count;
    int req_count = requests.size();
    if (req_count == 0) { return -1; }
    for (int req_idx = 0; req_idx < req_count; ++req_idx) {
        int dist = calc_port_distance(req.src_port_idx);
        if (least_dist >= dist) {
            res_req_idx = req_idx;
        }
    }
    return res_req_idx;
}
