#pragma once

#include "mesh_utils.hpp"
#include "../core/interconnect.hpp" ///< здесь уже все headers из core

class MeshInterconnect : public Interconnect {
private:
    std::vector<std::unique_ptr<MeshRouter>> routers;
    MeshRouter* get_router(int x, int y);

public:
    MeshInterconnect(int w, int h) :
    Interconnect(w, h)
    { total_nodes = w * h; }

    void build() override;
    void link_ports(MeshRouter src, MeshDirection srcDir,
                    MeshRouter dst, MeshDirection dstDir);
    void on_clock() override;
    bool injectPacket(int srcNodeId, const Packet& pkt) override;
    bool ejectPacket(int dstNodeIdm Packet& out_pkt) override;
}
