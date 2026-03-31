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

//=============================================================================
// Тест полного build()
//=============================================================================
TEST_F(MeshInterconnectTest, BuildCreatesWorkingNetwork) {
    mesh_3x3->build();

    // ============================================================
    // ШАГ 1: Проверяем LOCAL порты у всех роутеров
    // ============================================================
    for (size_t i = 0; i < get_routers(*mesh_3x3).size(); ++i) {
        auto* router = get_routers(*mesh_3x3)[i].get();
        EXPECT_TRUE(router->has_in_port(MeshDirection::LOCAL))
            << "Router " << i << " missing LOCAL in port";
        EXPECT_TRUE(router->has_out_port(MeshDirection::LOCAL))
            << "Router " << i << " missing LOCAL out port";
    }

    // ============================================================
    // ШАГ 2: Проверяем соединения между роутерами 0 и 1
    // ============================================================
    auto* router_0 = get_routers(*mesh_3x3)[0].get();
    auto* router_1 = get_routers(*mesh_3x3)[1].get();

    // Проверяем наличие портов для соединения
    EXPECT_TRUE(router_0->has_out_port(MeshDirection::EAST))
        << "Router 0 missing EAST out port";
    EXPECT_TRUE(router_1->has_in_port(MeshDirection::WEST))
        << "Router 1 missing WEST in port";
    EXPECT_TRUE(router_1->has_out_port(MeshDirection::WEST))
        << "Router 1 missing WEST out port";
    EXPECT_TRUE(router_0->has_in_port(MeshDirection::EAST))
        << "Router 0 missing EAST in port";

    // Получаем порты для проверки связи
    Port* east_out_0 = router_0->get_out_port(MeshDirection::EAST);
    Port* west_in_1 = router_1->get_in_port(MeshDirection::WEST);
    Port* west_out_1 = router_1->get_out_port(MeshDirection::WEST);
    Port* east_in_0 = router_0->get_in_port(MeshDirection::EAST);

    // Проверяем, что порты соединены правильно
    EXPECT_EQ(east_out_0, west_in_1)
        << "EAST out of router 0 not connected to WEST in of router 1";
    EXPECT_EQ(west_out_1, east_in_0)
        << "WEST out of router 1 not connected to EAST in of router 0";

    // ============================================================
    // ШАГ 3: Отправляем пакет из роутера 0 в роутер 1
    // ============================================================
    Packet pkt(42, 0, 1);
    bool injected = mesh_3x3->inject_packet(0, pkt);
    ASSERT_TRUE(injected) << "Packet injection failed";

    // ============================================================
    // ШАГ 4: Проверяем, что пакет попал в LOCAL вход роутера 0
    // ============================================================
    Port* local_in_0 = router_0->get_in_port(MeshDirection::LOCAL);
    ASSERT_TRUE(local_in_0->has_data())
        << "Packet not in LOCAL input of router 0";

    auto peeked = local_in_0->peek();
    ASSERT_TRUE(peeked.has_value());
    EXPECT_EQ(peeked->id, 42);
    EXPECT_EQ(peeked->src, 0);
    EXPECT_EQ(peeked->dst, 1);
    EXPECT_EQ(peeked->current_hop, 0);

    // ============================================================
    // ШАГ 5: Первый такт - маршрутизация и передача
    // ============================================================
    mesh_3x3->on_clock();

    // Проверяем: пакет ушел из LOCAL входа
    EXPECT_FALSE(local_in_0->has_data())
        << "Packet still in LOCAL input after clock";

    // Проверяем: пакет появился на EAST выходе роутера 0
    ASSERT_TRUE(east_out_0->has_data())
        << "Packet not on EAST output of router 0";

    peeked = east_out_0->peek();
    ASSERT_TRUE(peeked.has_value());
    EXPECT_EQ(peeked->id, 42);
    EXPECT_EQ(peeked->dst, 1);

    // Проверяем: пакет еще НЕ дошел до роутера 1 (еще в канале)
    EXPECT_FALSE(west_in_1->has_data())
        << "Packet already in WEST input of router 1 (should be in flight)";

    // ============================================================
    // ШАГ 6: Второй такт - передача из канала в роутер 1
    // ============================================================
    mesh_3x3->on_clock();

    // Проверяем: пакет ушел с EAST выхода роутера 0
    EXPECT_FALSE(east_out_0->has_data())
        << "Packet still on EAST output after clock";

    // Проверяем: пакет появился на WEST входе роутера 1
    ASSERT_TRUE(west_in_1->has_data())
        << "Packet not on WEST input of router 1";

    peeked = west_in_1->peek();
    ASSERT_TRUE(peeked.has_value());
    EXPECT_EQ(peeked->id, 42);

    // ============================================================
    // ШАГ 7: Третий такт - маршрутизация внутри роутера 1 и доставка
    // ============================================================
    mesh_3x3->on_clock();

    // Проверяем: пакет ушел с WEST входа
    EXPECT_FALSE(west_in_1->has_data())
        << "Packet still on WEST input after clock";

    // Проверяем: пакет появился на LOCAL выходе роутера 1
    Port* local_out_1 = router_1->get_out_port(MeshDirection::LOCAL);
    ASSERT_TRUE(local_out_1->has_data())
        << "Packet not on LOCAL output of router 1";

    peeked = local_out_1->peek();
    ASSERT_TRUE(peeked.has_value());
    EXPECT_EQ(peeked->id, 42);

    // ============================================================
    // ШАГ 8: Извлекаем пакет из сети
    // ============================================================
    auto received = mesh_3x3->eject_packet(1);
    ASSERT_TRUE(received.has_value())
        << "Packet not delivered to destination";
    EXPECT_EQ(received->id, 42);
    EXPECT_EQ(received->src, 0);
    EXPECT_EQ(received->dst, 1);

    // Проверяем, что LOCAL выход очистился после извлечения
    EXPECT_FALSE(local_out_1->has_data())
        << "Packet still on LOCAL output after eject";
}
