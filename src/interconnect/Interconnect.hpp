#pragma once

#include "router.hpp"

#include <string>
#include <nlohmann/json.hpp>

struct SimStats {
    double avgLatency;
    double maxLatency;
    double throughput;
    int packetsDelivered;
    int packetsLost;
};

enum class TopologyType { MESH, BUTTERFLY, UNKNOwN};
enum class RoutingType  { OBLIVIOUS, ADAPTIVE, UNKNOWN};

class Node {

public:
    void clock();
}

class Interconnect {
private:
    std::vector<Node&> nodes_;
    TopologyType topology_;
    RoutingType  routing_;
    int width_;
    int length_;
    int total_nodes_;


    virtual void clock();
public:

    virtual ~Interconnect();

    virtual bool buildTopology();
    virtual int  route_pkg(Packet& pkg, int current_node_idx);
    virtual SimStats run_simulation();
};
