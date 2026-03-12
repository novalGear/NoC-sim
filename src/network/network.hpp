#pragma once

#include <string>

class Node {

public:
    void tick();
}

class Network {
private:
    std::vector<Node&> nodes_;
    TopologyType type_;
    size_t width_;
    size_t length_;

public:
    void tick();
}
