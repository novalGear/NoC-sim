#include "mesh.hpp"
#include "mesh_router.hpp"
#include "mesh_utils.hpp"

#include <cassert>

MeshRouter* MeshInterconnect::get_router(size_t node_idx) const {
    assert(node_idx < routers_.size());
    return routers_[node_idx].get();
}

MeshRouter* MeshInterconnect::get_router(int x, int y) const {
    MeshCoords coords = {x, y};
    if (!coords.is_valid(width_, height_)) {
        return nullptr;
    }
    int idx = coords2id(coords, width_, height_);
    assert(idx >= 0 && idx < static_cast<int>(routers_.size()));
    return routers_[idx].get();
}

Port* MeshInterconnect::create_port() {
    auto port = std::make_unique<Port>();
    Port* raw_ptr = port.get();
    all_ports_.push_back(std::move(port));
    return raw_ptr;     // остается валидным при реаллокациях all_ports_
}

void MeshInterconnect::build() {
    init_routers();
    link_all_routers();
}

void MeshInterconnect::init_routers() {
    routers_.clear();
    routers_.reserve(total_nodes_);

    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            size_t node_id = coords2id(x, y, width_, height_);
            auto router = std::make_unique<MeshRouter>(node_id, x, y, width_, height_);
            routers_.push_back(std::move(router));

            reg_local_ports(node_id);
        }
    }
}

void MeshInterconnect::link_all_routers() {
    for (size_t node_idx = 0; node_idx < total_nodes_; ++node_idx) {
        if (has_neighbor(node_idx, width_, height_, MeshDirection::NORTH)) {
            link_routers(node_idx, MeshDirection::NORTH);
        }
        if (has_neighbor(node_idx, width_, height_, MeshDirection::EAST)) {
            link_routers(node_idx, MeshDirection::EAST);
        }
    }
}

void MeshInterconnect::link_routers(size_t node_idx, MeshDirection dir_1_to_2) {
    MeshRouter* router_1 = get_router(node_idx);
    assert(router_1);

    int router_2_idx = get_neighbor_id(node_idx, width_, height_, dir_1_to_2);
    if (router_2_idx != -1) {
        MeshDirection dir_2_to_1 = opposite(dir_1_to_2);
        MeshRouter* router_2 = get_router(router_2_idx);
        assert(router_2);

        Port* port_1_to_2  = create_port();
        Port* port_2_to_1  = create_port();

        router_1->register_in_port (dir_1_to_2, port_2_to_1);
        router_1->register_out_port(dir_1_to_2, port_1_to_2);

        router_2->register_in_port (dir_2_to_1, port_1_to_2);
        router_2->register_out_port(dir_2_to_1, port_2_to_1);
    }
}

void MeshInterconnect::reg_local_ports(int node_idx) {
    MeshRouter* router = get_router(node_idx);
    assert(router);
    Port* in_port  = create_port();
    Port* out_port = create_port();

    router->register_in_port(MeshDirection::LOCAL, in_port);
    router->register_out_port(MeshDirection::LOCAL, out_port);
}

bool MeshInterconnect::inject_packet(int src_node_idx, const Packet& pkt) {
    MeshRouter* router = get_router(src_node_idx);
    assert(router);
    return router->inject_packet(pkt);
}

std::optional<Packet> MeshInterconnect::eject_packet(int dst_node_idx) {
    MeshRouter* router = get_router(dst_node_idx);
    assert(router);
    return router->eject_packet();
}
