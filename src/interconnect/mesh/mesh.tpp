/**
@file mesh.tpp
@brief Implementation of the templated MeshInterconnect class.
*/
#pragma once
#include "mesh.hpp"
#include "mesh_router.hpp"
#include "mesh_utils.hpp"

template <typename ArbiterT>
MeshInterconnect<ArbiterT>::MeshInterconnect(int w, int h, const std::string & routing_algo)
    : Interconnect<ArbiterT>(w * h, routing_algo)
    , width_(w)
    , height_(h)
    , total_nodes_(w * h)
{
    assert(w > 0 && "Width must be positive");
    assert(h > 0 && "Height must be positive");
}

template <typename ArbiterT>
MeshRouter<ArbiterT>* MeshInterconnect<ArbiterT>::get_router(size_t node_idx) const {
    assert(node_idx < this->routers_.size());
    return static_cast<MeshRouter<ArbiterT>*>(this->routers_[node_idx].get());
}

template <typename ArbiterT>
MeshRouter<ArbiterT>* MeshInterconnect<ArbiterT>::get_router(int x, int y) const {
    MeshCoords coords = {x, y};
    if (!coords.is_valid(width_, height_)) {
        return nullptr;
    }
    int idx = coords2id(coords, width_, height_);
    assert(idx >= 0 && idx < static_cast<int>(this->routers_.size()));
    return static_cast<MeshRouter<ArbiterT>*>(this->routers_[idx].get());
}

template <typename ArbiterT>
Port* MeshInterconnect<ArbiterT>::create_port() {
    auto port = std::make_unique<Port>();
    Port* raw_ptr = port.get();
    this->all_ports_.push_back(std::move(port));
    return raw_ptr;
}

template <typename ArbiterT>
void MeshInterconnect<ArbiterT>::build() {
    init_routers();
    link_all_routers();
}

template <typename ArbiterT>
void MeshInterconnect<ArbiterT>::init_routers() {
    this->routers_.clear();
    this->routers_.reserve(total_nodes_);
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            size_t node_id = coords2id(x, y, width_, height_);
            auto router = std::make_unique<MeshRouter<ArbiterT>>(node_id, x, y, width_, height_);
            this->routers_.push_back(std::move(router));
            reg_local_ports(node_id);
        }
    }
}

template <typename ArbiterT>
void MeshInterconnect<ArbiterT>::link_all_routers() {
    for (size_t node_idx = 0; node_idx < total_nodes_; ++node_idx) {
        if (has_neighbor(node_idx, width_, height_, MeshDirection::NORTH)) {
            link_routers(node_idx, MeshDirection::NORTH);
        }
        if (has_neighbor(node_idx, width_, height_, MeshDirection::EAST)) {
            link_routers(node_idx, MeshDirection::EAST);
        }
    }
}

template <typename ArbiterT>
void MeshInterconnect<ArbiterT>::link_routers(size_t node_idx, MeshDirection dir_1_to_2) {
    MeshRouter<ArbiterT>* router_1 = this->get_router(node_idx);
    assert(router_1);

    int router_2_idx = get_neighbor_id(node_idx, width_, height_, dir_1_to_2);
    if (router_2_idx != -1) {
        MeshDirection dir_2_to_1 = opposite(dir_1_to_2);
        MeshRouter<ArbiterT>* router_2 = this->get_router(router_2_idx);
        assert(router_2);

        Port* port_1_to_2  = create_port();
        Port* port_2_to_1  = create_port();

        router_1->register_in_port (dir_1_to_2, port_2_to_1);
        router_1->register_out_port(dir_1_to_2, port_1_to_2);

        router_2->register_in_port (dir_2_to_1, port_1_to_2);
        router_2->register_out_port(dir_2_to_1, port_2_to_1);
    }
}

template <typename ArbiterT>
void MeshInterconnect<ArbiterT>::reg_local_ports(int node_idx) {
    MeshRouter<ArbiterT>* router = this->get_router(node_idx);
    assert(router);
    Port* in_port  = create_port();
    Port* out_port = create_port();
    router->register_in_port(MeshDirection::LOCAL, in_port);
    router->register_out_port(MeshDirection::LOCAL, out_port);
}

template <typename ArbiterT>
void MeshInterconnect<ArbiterT>::on_clock() {
    for (auto & router : this->routers_) {
        router->on_clock();
    }

    for (auto & port : this->all_ports_) {
        port->on_clock();
    }
}

template <typename ArbiterT>
bool MeshInterconnect<ArbiterT>::inject_packet(int src_node_idx, const Packet& pkt) {
    MeshRouter<ArbiterT>* router = this->get_router(src_node_idx);
    assert(router);
    return router->inject_packet(pkt);
}

template <typename ArbiterT>
std::optional<Packet> MeshInterconnect<ArbiterT>::eject_packet(int dst_node_idx) {
    MeshRouter<ArbiterT>* router = this->get_router(dst_node_idx);
    assert(router);
    return router->eject_packet();
}
