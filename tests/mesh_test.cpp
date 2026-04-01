/**
 * @file mesh_test.cpp
 * @brief Google Test для MeshInterconnect (тестирование приватных методов).
 * @author Novoselov Alexander
 * @date 18/03/2026
 */

#include <gtest/gtest.h>
#include "mesh.hpp"
#include "mesh_router.hpp"

// Дружественный тестовый класс
class MeshInterconnectTest : public ::testing::Test {
protected:
    void SetUp() override {
        mesh_3x3 = std::make_unique<MeshInterconnect>(3, 3);
        mesh_1x5 = std::make_unique<MeshInterconnect>(1, 5);
    }

    std::unique_ptr<MeshInterconnect> mesh_3x3;
    std::unique_ptr<MeshInterconnect> mesh_1x5;

    // Вспомогательные методы для доступа к приватным членам
    MeshRouter* get_router(MeshInterconnect& mesh, size_t idx) {
        return mesh.get_router(idx);
    }

    const auto& get_routers(const MeshInterconnect& mesh) {
        return mesh.routers_;
    }

    const auto& get_all_ports(const MeshInterconnect& mesh) {
        return mesh.all_ports_;
    }

    void call_init_routers(MeshInterconnect& mesh) {
        mesh.init_routers();
    }

    void call_reg_local_ports(MeshInterconnect& mesh, int node_idx) {
        mesh.reg_local_ports(node_idx);
    }

    void call_link_all_routers(MeshInterconnect& mesh) {
        mesh.link_all_routers();
    }

    void call_link_routers(MeshInterconnect& mesh, size_t node_idx, MeshDirection dir) {
        mesh.link_routers(node_idx, dir);
    }

    Port* call_create_port(MeshInterconnect& mesh) {
        return mesh.create_port();
    }
};

//=============================================================================
// Тесты приватных методов
//=============================================================================

TEST_F(MeshInterconnectTest, CreatePortAddsToAllPorts) {
    size_t initial_size = get_all_ports(*mesh_3x3).size();

    Port* port = call_create_port(*mesh_3x3);

    EXPECT_NE(port, nullptr);
    EXPECT_EQ(get_all_ports(*mesh_3x3).size(), initial_size + 1);
    EXPECT_EQ(get_all_ports(*mesh_3x3).back().get(), port);
}

TEST_F(MeshInterconnectTest, InitRoutersCreatesCorrectCount) {
    call_init_routers(*mesh_3x3);

    EXPECT_EQ(get_routers(*mesh_3x3).size(), 9);

    for (size_t i = 0; i < get_routers(*mesh_3x3).size(); ++i) {
        ASSERT_NE(get_routers(*mesh_3x3)[i], nullptr);
        EXPECT_EQ(get_routers(*mesh_3x3)[i]->get_id(), static_cast<int>(i));
    }
}

TEST_F(MeshInterconnectTest, InitRoutersCreatesCorrectCoordinates) {
    call_init_routers(*mesh_3x3);

    auto* router_0 = get_routers(*mesh_3x3)[0].get();
    EXPECT_EQ(router_0->get_coords().x, 0);
    EXPECT_EQ(router_0->get_coords().y, 0);

    auto* router_4 = get_routers(*mesh_3x3)[4].get();
    EXPECT_EQ(router_4->get_coords().x, 1);
    EXPECT_EQ(router_4->get_coords().y, 1);

    auto* router_8 = get_routers(*mesh_3x3)[8].get();
    EXPECT_EQ(router_8->get_coords().x, 2);
    EXPECT_EQ(router_8->get_coords().y, 2);
}

TEST_F(MeshInterconnectTest, InitRoutersCreatesLocalPorts) {
    call_init_routers(*mesh_3x3);

    // После init_routers LOCAL порты уже должны быть зарегистрированы
    for (const auto& router : get_routers(*mesh_3x3)) {
        EXPECT_TRUE(router->has_in_port(MeshDirection::LOCAL));
        EXPECT_TRUE(router->has_out_port(MeshDirection::LOCAL));
    }
}

TEST_F(MeshInterconnectTest, InitRoutersNoOtherPortsYet) {
    call_init_routers(*mesh_3x3);

    // После init_routers нет соединений между роутерами
    auto* router_0 = get_routers(*mesh_3x3)[0].get();
    EXPECT_FALSE(router_0->has_in_port(MeshDirection::EAST));
    EXPECT_FALSE(router_0->has_out_port(MeshDirection::EAST));
    EXPECT_FALSE(router_0->has_in_port(MeshDirection::NORTH));
    EXPECT_FALSE(router_0->has_out_port(MeshDirection::NORTH));
    EXPECT_FALSE(router_0->has_in_port(MeshDirection::SOUTH));
    EXPECT_FALSE(router_0->has_out_port(MeshDirection::SOUTH));
    EXPECT_FALSE(router_0->has_in_port(MeshDirection::WEST));
    EXPECT_FALSE(router_0->has_out_port(MeshDirection::WEST));
}

TEST_F(MeshInterconnectTest, LinkRoutersConnectsNeighbors) {
    // Создаем роутеры с LOCAL портами
    call_init_routers(*mesh_3x3);

    auto* router_0 = get_routers(*mesh_3x3)[0].get();

    // До соединения нет портов EAST/NORTH
    EXPECT_FALSE(router_0->has_in_port(MeshDirection::EAST));
    EXPECT_FALSE(router_0->has_out_port(MeshDirection::EAST));
    EXPECT_FALSE(router_0->has_in_port(MeshDirection::NORTH));
    EXPECT_FALSE(router_0->has_out_port(MeshDirection::NORTH));

    // Соединяем роутеры
    call_link_all_routers(*mesh_3x3);

    // После соединения должны появиться порты
    EXPECT_TRUE(router_0->has_in_port(MeshDirection::EAST));
    EXPECT_TRUE(router_0->has_out_port(MeshDirection::EAST));
    EXPECT_TRUE(router_0->has_in_port(MeshDirection::NORTH));
    EXPECT_TRUE(router_0->has_out_port(MeshDirection::NORTH));

    // Для углового узла нет SOUTH и WEST
    EXPECT_FALSE(router_0->has_in_port(MeshDirection::SOUTH));
    EXPECT_FALSE(router_0->has_out_port(MeshDirection::SOUTH));
    EXPECT_FALSE(router_0->has_in_port(MeshDirection::WEST));
    EXPECT_FALSE(router_0->has_out_port(MeshDirection::WEST));
}

TEST_F(MeshInterconnectTest, LinkRoutersCenterNodeHasAllPorts) {
    call_init_routers(*mesh_3x3);
    call_link_all_routers(*mesh_3x3);

    // Центральный узел (1,1) должен иметь все 4 направления
    auto* center = get_routers(*mesh_3x3)[4].get();

    EXPECT_TRUE(center->has_in_port(MeshDirection::NORTH));
    EXPECT_TRUE(center->has_out_port(MeshDirection::NORTH));
    EXPECT_TRUE(center->has_in_port(MeshDirection::EAST));
    EXPECT_TRUE(center->has_out_port(MeshDirection::EAST));
    EXPECT_TRUE(center->has_in_port(MeshDirection::SOUTH));
    EXPECT_TRUE(center->has_out_port(MeshDirection::SOUTH));
    EXPECT_TRUE(center->has_in_port(MeshDirection::WEST));
    EXPECT_TRUE(center->has_out_port(MeshDirection::WEST));
}

TEST_F(MeshInterconnectTest, GetRouterReturnsCorrectRouter) {
    call_init_routers(*mesh_3x3);

    auto* router = get_router(*mesh_3x3, 4);
    ASSERT_NE(router, nullptr);
    EXPECT_EQ(router->get_id(), 4);
    EXPECT_EQ(router->get_coords().x, 1);
    EXPECT_EQ(router->get_coords().y, 1);
}

TEST_F(MeshInterconnectTest, BuildCreatesWorkingNetwork) {
    std::cout << "\n=== Test BuildCreatesWorkingNetwork ===" << std::endl;

    mesh_3x3->build();
    std::cout << "build() completed" << std::endl;

    // Отправляем пакет
    Packet pkt(42, 0, 1);
    bool injected = mesh_3x3->inject_packet(0, pkt);
    std::cout << "inject_packet returned: " << injected << std::endl;

    std::cout << "\n--- First on_clock() ---" << std::endl;
    mesh_3x3->on_clock();

    std::cout << "\n--- Second on_clock() ---" << std::endl;
    mesh_3x3->on_clock();

    auto received = mesh_3x3->eject_packet(1);
    std::cout << "eject_packet returned: " << (received.has_value() ? "value" : "nullopt") << std::endl;

    if (received.has_value()) {
        std::cout << "Packet id=" << received->id << " src=" << received->src << " dst=" << received->dst << std::endl;
    }

    ASSERT_TRUE(received.has_value()) << "Packet not delivered to destination";
    EXPECT_EQ(received->id, 42);
}
