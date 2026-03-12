#pragma once

#include "packet.hpp"
#include <optional>

class Port {
private:
    std::optional<Packet> buffer;
    void clr_buffer() { buffer.reset(); }

public:

    bool hasData() { return buffer.has_value();  }
    bool isReady() { return !buffer.has_value(); }

    bool trySend(Packet& pkt);
    Packet tryRecv();
};

bool Port::trySend(Packet& pkt) {
    if (hasData()) {
        buffer = pkt;
        return true;
    }
    return false;
}

std::optional<Packet> Port::tryRecv() {
    if (hasData()) {
        Packet pkt = buffer;
        clr_buffer();
        return pkt;
    }
    return std::nullopt;
}
