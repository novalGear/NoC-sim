#include "mesh_router.hpp"
#include "mesh_utils.hpp"

MeshRouter::MeshRouter(int id, int x, int y, int width, int height)
    : Router(MESH_PORTS_PER_ROUTER, MESH_PORTS_PER_ROUTER, id)
    , coords{x, y}
    , grid_width(width)
    , grid_height(height)
{
    assert(width  > 0 && "Width must be positive");
    assert(height > 0 && "Height must be positive");
    DEBUG_PRINT_COORDS(this);
}

int MeshRouter::route_pkt(const Packet& pkt) const {
    MeshCoords dst_coords = id2coords(pkt.dst, grid_width);
    MeshDirection dst_dir;
    DEBUG_PRINT_ROUTER(this, "route_pkt: from (" << coords.x << ","
                       << coords.y << ") to dst=" << pkt.dst << " ("
                       << dst_coords.x << "," << dst_coords.y << ")");
    // Сначала двигаемся вдоль оси x
    if (dst_coords.x > coords.x)      {
        dst_dir = MeshDirection::EAST;
        DEBUG_PRINT_ROUTER(this, "  -> EAST");
    }
    else if (dst_coords.x < coords.x) {
        dst_dir = MeshDirection::WEST;
        DEBUG_PRINT_ROUTER(this, "  -> WEST");
    }
    // Потом вдоль оси y
    else if (dst_coords.y > coords.y) {
        dst_dir = MeshDirection::NORTH;
        DEBUG_PRINT_ROUTER(this, "  -> NORTH");
    }
    else if (dst_coords.y < coords.y) {
        dst_dir = MeshDirection::SOUTH;
        DEBUG_PRINT_ROUTER(this, "  -> SOUTH");
    }
    // Прибыли в конечный пункт
    else {
        dst_dir = MeshDirection::LOCAL;
        DEBUG_PRINT_ROUTER(this, "  -> LOCAL");
    }

    assert(has_out_port(dst_dir) && "No such output port");
    return static_cast<int>(dst_dir);
}

Port* MeshRouter::get_in_port(MeshDirection dir) {
    assert(has_in_port(dir) && "No such port registered");
    size_t port_idx = static_cast<size_t>(dir);
    assert(port_idx < input_ports.size() && "Index out of boundaries");
    return input_ports[port_idx];
}

Port* MeshRouter::get_out_port(MeshDirection dir) {
    assert(has_out_port(dir) && "No such port registered");
    size_t port_idx = static_cast<size_t>(dir);
    assert(port_idx < output_ports.size() && "Index out of boundaries");
    return output_ports[port_idx];
}

void MeshRouter::register_in_port(MeshDirection dir, Port* port) {
    assert(!has_in_port(dir) && "In port override");
    assert(port);

    size_t idx = static_cast<size_t>(dir);
    assert(idx < input_ports.size() && "Index out of boundaries");

    input_ports[idx] = port;
    set_in_port_exists(idx);
}

void MeshRouter::register_out_port(MeshDirection dir, Port* port) {
    assert(!has_out_port(dir) && "Out port override");
    assert(port);

    size_t idx = static_cast<size_t>(dir);
    assert(idx < output_ports.size() && "Index out of boundaries");

    output_ports[idx] = port;
    set_out_port_exists(idx);
}

bool MeshRouter::inject_packet(const Packet& pkt) {
    DEBUG_PRINT_ROUTER(this, "inject_packet: id=" << pkt.id << " dst=" << pkt.dst);
    assert(has_in_port(MeshDirection::LOCAL));
    Port* local_in_port = get_in_port(MeshDirection::LOCAL);
    assert(local_in_port);
    return local_in_port->try_send(pkt);
}

std::optional<Packet> MeshRouter::eject_packet() {
    DEBUG_PRINT_ROUTER(this, "eject_packet");
    assert(has_out_port(MeshDirection::LOCAL));
    Port* local_out_port = get_out_port(MeshDirection::LOCAL);
    assert(local_out_port);
    return local_out_port->try_recv();
}
