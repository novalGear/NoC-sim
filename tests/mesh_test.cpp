/**
@file mesh_test.cpp
@brief Google Test для MeshInterconnect.
@author Novoselov Alexander
@date 18/03/2026
@updated 10/05/2026 - Fixed friend access and MeshDirection casts
*/
#include <gtest/gtest.h>
#include "mesh.hpp"
#include "mesh_router.hpp"
#include "rr_arbiter.hpp"

class MeshInterconnectTest : public ::testing::Test {
protected:
    void SetUp() override {
        mesh_3x3 = std::make_unique<MeshInterconnect<RRArbiter>>(3, 3);
        mesh_1x5 = std::make_unique<MeshInterconnect<RRArbiter>>(1, 5);
    }

    std::unique_ptr<MeshInterconnect<RRArbiter>> mesh_3x3;
    std::unique_ptr<MeshInterconnect<RRArbiter>> mesh_1x5;

    // Хелперы теперь работают, так как Interconnect объявил friend class MeshInterconnectTest
    const auto& get_routers(const MeshInterconnect<RRArbiter>& mesh) {
        return mesh.routers_;
    }
    const auto& get_all_ports(const MeshInterconnect<RRArbiter>& mesh) {
        return mesh.all_ports_;
    }
    MeshRouter<RRArbiter>* get_router(MeshInterconnect<RRArbiter>& mesh, size_t idx) {
        return static_cast<MeshRouter<RRArbiter>*>(mesh.routers_[idx].get());
    }
    void call_init_routers(MeshInterconnect<RRArbiter>& mesh) { mesh.init_routers(); }
    void call_link_all_routers(MeshInterconnect<RRArbiter>& mesh) { mesh.link_all_routers(); }
    Port* call_create_port(MeshInterconnect<RRArbiter>& mesh) { return mesh.create_port(); }
};

TEST_F(MeshInterconnectTest, CreatePortAddsToAllPorts) {
    size_t initial_size = get_all_ports(*mesh_3x3).size();
    Port* port = call_create_port(*mesh_3x3);
    EXPECT_NE(port, nullptr);
    EXPECT_EQ(get_all_ports(*mesh_3x3).size(), initial_size + 1);
}

TEST_F(MeshInterconnectTest, InitRoutersCreatesCorrectCoordinates) {
    call_init_routers(*mesh_3x3);
    auto* router_0 = static_cast<MeshRouter<RRArbiter>*>(get_routers(*mesh_3x3)[0].get());
    EXPECT_EQ(router_0->get_coords().x, 0);
    EXPECT_EQ(router_0->get_coords().y, 0);

    auto* router_4 = static_cast<MeshRouter<RRArbiter>*>(get_routers(*mesh_3x3)[4].get());
    EXPECT_EQ(router_4->get_coords().x, 1);
    EXPECT_EQ(router_4->get_coords().y, 1);
}

TEST_F(MeshInterconnectTest, LinkRoutersConnectsNeighbors) {
    call_init_routers(*mesh_3x3);
    auto* router_0 = get_routers(*mesh_3x3)[0].get();

    EXPECT_FALSE(router_0->has_in_port(static_cast<int>(MeshDirection::EAST)));
    EXPECT_FALSE(router_0->has_out_port(static_cast<int>(MeshDirection::EAST)));

    call_link_all_routers(*mesh_3x3);

    EXPECT_TRUE(router_0->has_in_port(static_cast<int>(MeshDirection::EAST)));
    EXPECT_TRUE(router_0->has_out_port(static_cast<int>(MeshDirection::EAST)));
}

TEST_F(MeshInterconnectTest, BuildCreatesWorkingNetwork) {
    mesh_3x3->build();
    Packet pkt(42, 0, 1);
    EXPECT_TRUE(mesh_3x3->inject_packet(0, pkt));
    mesh_3x3->on_clock();
    mesh_3x3->on_clock();
    auto received = mesh_3x3->eject_packet(1);
    ASSERT_TRUE(received.has_value());
    EXPECT_EQ(received->id, 42);
}
