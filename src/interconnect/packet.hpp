#pragma once

#include <vector>

struct Packet {
    int id;
    int src_node;
    int dst_node;
    int hop_count;
};

struct Request {
    int src_port_idx;
};

using RequestList = std::vector<Request>;
using AllRequests = std::vector<RequestList>;
