#include "mesh.hpp"
#include "mesh_router.hpp"
#include "mesh_utils.hpp"

#include <cassert>

MeshRouter* MshInterconnect::get_router(size_t node_idx) {
    assert(node_idx < routers_.size());
    return routers_[node_idx].get();
}

MeshRouter* MeshInterconnect::get_router(int x, int y) const {
    MeshCoords coords = {x, y};
    if (!coords.is_valid(width_, height_)) {
        return nullptr;
    }
    int idx = coords2id(coords, width_);
    assert(idx >= 0 && idx < static_cast<int>(routers_.size()));
    return routers_[idx].get();
}

Port* MeshInterconnect::create_port() {
    auto port = std::make_unique<Port>();
    Port* raw_ptr = port.get();
    all_ports_.pushback(std::move(port));
    return raw_ptr;     // остается валидным при реаллокациях all_ports_
}

void MeshInterconnect::build() {
    init_routers();
    link_all_routers();
}

void MeshInterconnect::init_routers() {
    routers_.clear();
    routers_.reserve(total_nodes_);

    for (size_t y = 0; y < height_; ++y) {
        for (size_t x = 0; x < width_; ++x) {
            size_t node_id = coords2id(x, y, width_);
            auto router = std::make_unique<MeshRouter>(node_id, x, y, width_, height_);
            routers_.push_back(std::move(router));
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

void MeshInterconnect::link_routers(size_t node_idx, MeshDirection 1_to_2_dir) {
    MeshRouter* router_1 = get_router(node_idx);

    int router_2_idx = get_neighbor_id(node_idx, width_, height_, 1_to_2_dir);
    if (router_2_idx != -1) {
        MeshDirection 2_to_1_dir = opposite(1_to_2_dir);
        MeshRouter* router_2 = get_router(router_2_idx);

        Port* 1_to_2_port  = create_port();
        Port* 2_to_1_port  = create_port();

        router_1->register_in_port (1_to_2_dir, 2_to_1_port);
        router_1->register_out_port(1_to_2_dir, 1_to_2_port);

        router_2->register_in_port (2_to_1_dir, 1_to_2_port);
        router_2->register_out_port(2_to_1_dir, 2_to_1_port);
    }
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
