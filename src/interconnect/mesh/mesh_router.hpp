#pragma once
#include "mesh_utils.hpp"
#include "../core/router.hpp" // Базовый класс Router
#include <cmath>

class MeshRouter : public Router {
private:
    int x_coord;
    int y_coord;
    int grid_width;

public:
    MeshRouter(int id, int x, int y, int w)
        : Router(MESH_IN_PORT_COUNT, MESH_OUT_PORT_COUNT, id),
          x_coord(x),
          y_coord(y),
          grid_width(w)
    {}

    int route_pkt(const Packet& pkt) override {
        MeshCoords dst_coords = id2coords(pkt.dst, grid_width);

        // Сначала двигаемся по X
        if (dst_coords.x > x_coord) return MeshDirection::EAST;
        if (dst_coords.x < x_coord) return MeshDirection::WEST;

        // Затем по Y
        if (dst_coords.y > y_coord) return MeshDirection::SOUTH;
        if (dst_coords.y < y_coord) return MeshDirection::NORTH;

        // Достигли назначения
        return MeshDirection::LOCAL_OUT;
    }

    // Геттеры координат (могут понадобиться при сборке сети)
    int getX() const { return x_coord; }
    int getY() const { return y_coord; }
};
