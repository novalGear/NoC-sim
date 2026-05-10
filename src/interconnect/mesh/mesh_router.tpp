#pragma once
#include "mesh_router.hpp"
#include "mesh_utils.hpp"

template <typename ArbiterT>
MeshRouter<ArbiterT>::MeshRouter(int id, int x, int y, int width, int height)
    : Router<ArbiterT>(5, 5, id), // MESH_PORTS_PER_ROUTER usually 5
      coords{x, y},
      grid_width(width),
      grid_height(height)
{
    assert(width > 0 && "Width must be positive");
    assert(height > 0 && "Height must be positive");
    // DEBUG_PRINT_COORDS(this); // Requires proper template support in debug macros
}

template <typename ArbiterT>
int MeshRouter<ArbiterT>::route_pkt(const Packet& pkt) const {
    MeshCoords dst_coords = id2coords(pkt.dst, grid_width);
    MeshDirection dst_dir;

    // DEBUG_PRINT_ROUTER(this, "route_pkt: from (" << coords.x << ","
    // << coords.y << ") to dst=" << pkt.dst << " ("
    // << dst_coords.x << "," << dst_coords.y << ")");

    if (dst_coords.x > coords.x)      {
        dst_dir = MeshDirection::EAST;
        // DEBUG_PRINT_ROUTER(this, "  -> EAST");
    }
    else if (dst_coords.x < coords.x) {
        dst_dir = MeshDirection::WEST;
        // DEBUG_PRINT_ROUTER(this, "  -> WEST");
    }
    else if (dst_coords.y > coords.y) {
        dst_dir = MeshDirection::NORTH; // Check your coordinate system: Y+ is usually North or South?
        // DEBUG_PRINT_ROUTER(this, "  -> NORTH");
    }
    else if (dst_coords.y < coords.y) {
        dst_dir = MeshDirection::SOUTH;
        // DEBUG_PRINT_ROUTER(this, "  -> SOUTH");
    }
    else {
        dst_dir = MeshDirection::LOCAL;
        // DEBUG_PRINT_ROUTER(this, "  -> LOCAL");
    }

    assert(this->has_out_port(dst_dir) && "No such output port");
    return static_cast<int>(dst_dir);
}

template <typename ArbiterT>
Port* MeshRouter<ArbiterT>::get_in_port(MeshDirection dir) {
    assert(this->has_in_port(dir) && "No such port registered");
    size_t port_idx = static_cast<size_t>(dir);
    assert(port_idx < this->input_ports.size() && "Index out of boundaries");
    return this->input_ports[port_idx];
}

template <typename ArbiterT>
Port* MeshRouter<ArbiterT>::get_out_port(MeshDirection dir) {
    assert(this->has_out_port(dir) && "No such port registered");
    size_t port_idx = static_cast<size_t>(dir);
    assert(port_idx < this->output_ports.size() && "Index out of boundaries");
    return this->output_ports[port_idx];
}

template <typename ArbiterT>
void MeshRouter<ArbiterT>::register_in_port(MeshDirection dir, Port* port) {
    assert(!this->has_in_port(dir) && "In port override");
    assert(port);
    size_t idx = static_cast<size_t>(dir);
    assert(idx < this->input_ports.size() && "Index out of boundaries");
    this->input_ports[idx] = port;
    this->set_in_port_exists(idx);
}

template <typename ArbiterT>
void MeshRouter<ArbiterT>::register_out_port(MeshDirection dir, Port* port) {
    assert(!this->has_out_port(dir) && "Out port override");
    assert(port);
    size_t idx = static_cast<size_t>(dir);
    assert(idx < this->output_ports.size() && "Index out of boundaries");
    this->output_ports[idx] = port;
    this->set_out_port_exists(idx);
}

template <typename ArbiterT>
bool MeshRouter<ArbiterT>::inject_packet(const Packet& pkt) {
    // DEBUG_PRINT_ROUTER(this, "inject_packet: id=" << pkt.id << " dst=" << pkt.dst);
    assert(this->has_in_port(MeshDirection::LOCAL));
    Port* local_in_port = get_in_port(MeshDirection::LOCAL);
    assert(local_in_port);
    return local_in_port->try_send(pkt);
}

template <typename ArbiterT>
std::optional<Packet> MeshRouter<ArbiterT>::eject_packet() {
    // DEBUG_PRINT_ROUTER(this, "eject_packet");
    assert(this->has_out_port(MeshDirection::LOCAL));
    Port* local_out_port = get_out_port(MeshDirection::LOCAL);
    assert(local_out_port);
    return local_out_port->try_recv();
}
