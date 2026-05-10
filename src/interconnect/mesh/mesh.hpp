/**
@file mesh.hpp
@brief Реализация Mesh топологии интерконнекта.
@author Novoselov Alexander
@date 16/03/2026
*/
#pragma once
#include "mesh_router.hpp"
#include "interconnect.hpp"
#include "mesh_utils.hpp"
#include <vector>
#include <memory>
#include <string>
#include <cassert>

class MeshInterconnectTest;

/**
@class MeshInterconnect
@brief Интерконнект с топологией двумерной сетки (Mesh).
*/
template <typename ArbiterT = RRArbiter>
class MeshInterconnect final : public Interconnect<ArbiterT> {

private:
    friend class MeshInterconnectTest;

    int width_;   ///< Ширина сетки
    int height_;  ///< Высота сетки
    int total_nodes_;

    // Helper methods declarations
    [[nodiscard]] MeshRouter<ArbiterT>* get_router(int x, int y) const;
    [[nodiscard]] MeshRouter<ArbiterT>* get_router(size_t node_idx) const;

    [[nodiscard]] Port* create_port();
    void init_routers();
    void reg_local_ports(int node_idx);
    void link_all_routers();
    void link_routers(size_t node_idx, MeshDirection dir_1_to_2);

public:
    /**
    * @brief Конструктор MeshInterconnect.
    */
    MeshInterconnect(int w, int h, const std::string & routing_algo = "DOR");

    ~MeshInterconnect() override = default;
    MeshInterconnect(const MeshInterconnect &) = delete;
    MeshInterconnect & operator=(const MeshInterconnect &) = delete;
    MeshInterconnect(MeshInterconnect &&) noexcept = default;
    MeshInterconnect & operator=(MeshInterconnect &&) noexcept = default;

    void build() override;
    void on_clock() override;
    bool inject_packet(int src_node_idx, const Packet & pkt) override;
    std::optional<Packet> eject_packet(int dst_node_idx) override;

    [[nodiscard]] int get_width() const { return width_; }
    [[nodiscard]] int get_height() const { return height_; }
};

// Include template implementations
#include "mesh.tpp"
