#pragma once

enum class MeshDirection {
    NORTH     = 0,
    SOUTH     = 1,
    EAST      = 2,
    WEST      = 3,
    LOCAL_IN  = 4,
    LOCAL_OUT = 5,
    COUNT
};

constexpr int MESH_PORT_COUNT = static_cast<int>(MeshDirection::COUNT);
constexpr int MESH_IN_PORT_COUNT = 1;
constexpr int MESH_OUT_PORT_COUNT = MESH_PORT_COUNT - MESH_IN_PORT_COUNT;

struct MeshCoords {
    int x;
    int y;
}

static inline MeshCoords id2coords(int nodeId, int width) {
    return {nodeId % width, nodeId / width};
}

static inline int coords2id(int x, int y, int width) {
    return y * width + x;
}
