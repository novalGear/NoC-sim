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
}

int MeshRouter::route_pkt(const Packet& pkt) const {
    MeshCoords dst_coords = id2coords(pkt.dst, grid_width);
    MeshDirection dst_dir;
    // Сначала двигаемся вдоль оси x
    if (dst_coords.x > coords.x)      { dst_dir = MeshDirection::EAST; }
    else if (dst_coords.x < coords.x) { dst_dir = MeshDirection::WEST; }
    // Потом вдоль оси y
    else if (dst_coords.y > coords.y) { dst_dir = MeshDirection::NORTH; }
    else if (dst_coords.y < coords.y) { dst_dir = MeshDirection::SOUTH; }
    // Прибыли в конечный пункт
    else { dst_dir = MeshDirection::LOCAL; }

    assert(has_out_port(dst_dir) && "No such output port");
    return static_cast<int>(dst_dir);
}

void MeshRouter::register_in_port(MeshDirection dir, Port* port) {
    assert(!has_in_port(dir) && "In port override");
    assert(port);

    size_t idx = static_cast<size_t>(dir);
    assert(idx < input_ports.size() && "Index out of boundaries");

    input_ports[idx] = port;
    in_ports_mask.set(idx);
}

void MeshRouter::register_out_port(MeshDirection dir, Port* port) {
    assert(!has_out_port(dir) && "Out port override");
    assert(port);

    size_t idx = static_cast<size_t>(dir);
    assert(idx < output_ports.size() && "Index out of boundaries");

    output_ports[idx] = port;
    out_ports_mask.set(idx);
}
