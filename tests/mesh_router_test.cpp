/**
 * @file mesh_router_test.cpp
 * @brief Google Test для MeshRouter.
 * @author Novoselov Alexander
 * @date 18/03/2026
 */

#include <gtest/gtest.h>
#include "mesh_router.hpp"
#include "mesh_utils.hpp"

class MeshRouterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создаем роутеры для разных позиций в сетке 3x3
        center_router = std::make_unique<MeshRouter>(4, 1, 1, 3, 3);
        corner_router = std::make_unique<MeshRouter>(0, 0, 0, 3, 3);
        edge_router = std::make_unique<MeshRouter>(1, 1, 0, 3, 3);

        // Создаем роутеры для линейных сеток
        horiz_router = std::make_unique<MeshRouter>(2, 2, 0, 5, 1);
        vert_router = std::make_unique<MeshRouter>(2, 0, 2, 1, 5);

        // Регистрируем ВСЕ порты для тестов маршрутизации
        register_all_ports(center_router.get());
        register_all_ports(corner_router.get());
        register_all_ports(edge_router.get());
        register_all_ports(horiz_router.get());
        register_all_ports(vert_router.get());
    }

    // Регистрируем все 5 портов для роутера
    void register_all_ports(MeshRouter* router) {
        for (int i = 0; i < MESH_PORTS_PER_ROUTER; ++i) {
            auto dir = static_cast<MeshDirection>(i);
            auto in = std::make_unique<Port>();
            auto out = std::make_unique<Port>();
            router->register_in_port(dir, in.get());
            router->register_out_port(dir, out.get());
            ports.push_back(std::move(in));
            ports.push_back(std::move(out));
        }
    }

    // Регистрируем только LOCAL порты
    // Регистрируем LOCAL порты и сохраняем в ports
    void register_local_ports(MeshRouter* router) {
        auto in = std::make_unique<Port>();
        auto out = std::make_unique<Port>();
        router->register_in_port(MeshDirection::LOCAL, in.get());
        router->register_out_port(MeshDirection::LOCAL, out.get());
        ports.push_back(std::move(in));
        ports.push_back(std::move(out));
    }

    std::unique_ptr<MeshRouter> center_router;
    std::unique_ptr<MeshRouter> corner_router;
    std::unique_ptr<MeshRouter> edge_router;
    std::unique_ptr<MeshRouter> horiz_router;
    std::unique_ptr<MeshRouter> vert_router;

    std::unique_ptr<Port> local_in;
    std::unique_ptr<Port> local_out;
    std::vector<std::unique_ptr<Port>> ports;
};

//=============================================================================
// Тесты инициализации
//=============================================================================

TEST_F(MeshRouterTest, ConstructorInitializes) {
    EXPECT_EQ(center_router->get_id(), 4);
    EXPECT_EQ(center_router->get_coords().x, 1);
    EXPECT_EQ(center_router->get_coords().y, 1);
    EXPECT_EQ(center_router->get_input_count(), 5);
    EXPECT_EQ(center_router->get_output_count(), 5);
}

TEST_F(MeshRouterTest, CornerRouterCoordinates) {
    EXPECT_EQ(corner_router->get_coords().x, 0);
    EXPECT_EQ(corner_router->get_coords().y, 0);
}

TEST_F(MeshRouterTest, EdgeRouterCoordinates) {
    EXPECT_EQ(edge_router->get_coords().x, 1);
    EXPECT_EQ(edge_router->get_coords().y, 0);
}

//=============================================================================
// Тесты регистрации портов
//=============================================================================

TEST_F(MeshRouterTest, RegisterAllPorts) {
    // Проверяем, что после register_all_ports все порты зарегистрированы
    for (int i = 0; i < MESH_PORTS_PER_ROUTER; ++i) {
        auto dir = static_cast<MeshDirection>(i);
        EXPECT_TRUE(center_router->has_in_port(dir))
            << "Missing in port for direction " << i;
        EXPECT_TRUE(center_router->has_out_port(dir))
            << "Missing out port for direction " << i;
        EXPECT_NE(center_router->get_in_port(dir), nullptr);
        EXPECT_NE(center_router->get_out_port(dir), nullptr);
    }
}

//=============================================================================
// Тесты X-Y маршрутизации (теперь с зарегистрированными портами)
//=============================================================================

TEST_F(MeshRouterTest, RouteToLocal) {
    Packet pkt(1, 4, 4);
    int out_port = center_router->route_pkt(pkt);
    EXPECT_EQ(out_port, static_cast<int>(MeshDirection::LOCAL));
}

TEST_F(MeshRouterTest, RouteEast) {
    Packet pkt(1, 4, 5);
    int out_port = center_router->route_pkt(pkt);
    EXPECT_EQ(out_port, static_cast<int>(MeshDirection::EAST));
}

TEST_F(MeshRouterTest, RouteWest) {
    Packet pkt(1, 4, 3);
    int out_port = center_router->route_pkt(pkt);
    EXPECT_EQ(out_port, static_cast<int>(MeshDirection::WEST));
}

TEST_F(MeshRouterTest, RouteNorth) {
    Packet pkt(1, 4, 7);
    int out_port = center_router->route_pkt(pkt);
    EXPECT_EQ(out_port, static_cast<int>(MeshDirection::NORTH));
}

TEST_F(MeshRouterTest, RouteSouth) {
    Packet pkt(1, 4, 1);
    int out_port = center_router->route_pkt(pkt);
    EXPECT_EQ(out_port, static_cast<int>(MeshDirection::SOUTH));
}

TEST_F(MeshRouterTest, RouteXThenY) {
    Packet pkt(1, 4, 8);
    int out_port = center_router->route_pkt(pkt);
    EXPECT_EQ(out_port, static_cast<int>(MeshDirection::EAST));
}

TEST_F(MeshRouterTest, RouteCornerNode) {
    Packet pkt(1, 0, 8);
    int out_port = corner_router->route_pkt(pkt);
    EXPECT_EQ(out_port, static_cast<int>(MeshDirection::EAST));

    pkt = Packet(1, 0, 0);
    out_port = corner_router->route_pkt(pkt);
    EXPECT_EQ(out_port, static_cast<int>(MeshDirection::LOCAL));
}

TEST_F(MeshRouterTest, RouteEdgeNode) {
    Packet pkt(1, 1, 7);
    int out_port = edge_router->route_pkt(pkt);
    EXPECT_EQ(out_port, static_cast<int>(MeshDirection::NORTH));

    pkt = Packet(1, 1, 2);
    out_port = edge_router->route_pkt(pkt);
    EXPECT_EQ(out_port, static_cast<int>(MeshDirection::EAST));
}

//=============================================================================
// Тесты горизонтальной сетки (5x1)
//=============================================================================

TEST_F(MeshRouterTest, HorizontalGridRouteEast) {
    Packet pkt(1, 2, 4);
    int out_port = horiz_router->route_pkt(pkt);
    EXPECT_EQ(out_port, static_cast<int>(MeshDirection::EAST));
}

TEST_F(MeshRouterTest, HorizontalGridRouteWest) {
    Packet pkt(1, 2, 0);
    int out_port = horiz_router->route_pkt(pkt);
    EXPECT_EQ(out_port, static_cast<int>(MeshDirection::WEST));
}

TEST_F(MeshRouterTest, HorizontalGridRouteLocal) {
    Packet pkt(1, 2, 2);
    int out_port = horiz_router->route_pkt(pkt);
    EXPECT_EQ(out_port, static_cast<int>(MeshDirection::LOCAL));
}

//=============================================================================
// Тесты вертикальной сетки (1x5)
//=============================================================================

TEST_F(MeshRouterTest, VerticalGridRouteNorth) {
    Packet pkt(1, 2, 4);
    int out_port = vert_router->route_pkt(pkt);
    EXPECT_EQ(out_port, static_cast<int>(MeshDirection::NORTH));
}

TEST_F(MeshRouterTest, VerticalGridRouteSouth) {
    Packet pkt(1, 2, 0);
    int out_port = vert_router->route_pkt(pkt);
    EXPECT_EQ(out_port, static_cast<int>(MeshDirection::SOUTH));
}

TEST_F(MeshRouterTest, VerticalGridRouteLocal) {
    Packet pkt(1, 2, 2);
    int out_port = vert_router->route_pkt(pkt);
    EXPECT_EQ(out_port, static_cast<int>(MeshDirection::LOCAL));
}

//=============================================================================
// Тесты inject_packet и eject_packet (только с LOCAL портами)
//=============================================================================

TEST_F(MeshRouterTest, InjectPacketToLocalPort) {
    // Создаем отдельный роутер только с LOCAL портами
    auto router = std::make_unique<MeshRouter>(4, 1, 1, 3, 3);
    register_local_ports(router.get());

    Packet pkt(42, 4, 4);
    bool injected = router->inject_packet(pkt);
    EXPECT_TRUE(injected);

    Port* local_in = router->get_in_port(MeshDirection::LOCAL);
    EXPECT_TRUE(local_in->has_data());

    auto peeked = local_in->peek();
    ASSERT_TRUE(peeked.has_value());
    EXPECT_EQ(peeked->id, 42);
}

TEST_F(MeshRouterTest, EjectPacketFromLocalPort) {
    auto router = std::make_unique<MeshRouter>(4, 1, 1, 3, 3);
    register_local_ports(router.get());

    Packet pkt(42, 4, 4);
    router->inject_packet(pkt);

    router->on_clock();

    auto ejected = router->eject_packet();
    ASSERT_TRUE(ejected.has_value());
    EXPECT_EQ(ejected->id, 42);

    Port* local_out = router->get_out_port(MeshDirection::LOCAL);
    EXPECT_FALSE(local_out->has_data());
}

//=============================================================================
// Тесты движения пакета через роутер
//=============================================================================

TEST_F(MeshRouterTest, PacketMovesFromLocalInToEastOut) {
    auto router = std::make_unique<MeshRouter>(0, 0, 0, 3, 3);

    // Регистрируем LOCAL порты
    register_local_ports(router.get());

    // Регистрируем EAST порты
    auto east_in = std::make_unique<Port>();
    auto east_out = std::make_unique<Port>();
    router->register_in_port(MeshDirection::EAST, east_in.get());
    router->register_out_port(MeshDirection::EAST, east_out.get());
    ports.push_back(std::move(east_in));
    ports.push_back(std::move(east_out));

    // Отправляем пакет в LOCAL вход
    Packet pkt(42, 0, 1);
    router->inject_packet(pkt);

    Port* local_in = router->get_in_port(MeshDirection::LOCAL);
    ASSERT_TRUE(local_in->has_data());

    // Обрабатываем такт
    router->on_clock();

    // Проверяем: пакет ушел из LOCAL входа
    EXPECT_FALSE(local_in->has_data());

    // Проверяем: пакет появился на EAST выходе
    Port* east_out_ptr = router->get_out_port(MeshDirection::EAST);
    EXPECT_TRUE(east_out_ptr->has_data());

    auto peeked = east_out_ptr->peek();
    ASSERT_TRUE(peeked.has_value());
    EXPECT_EQ(peeked->id, 42);
}

//=============================================================================
// Тесты backpressure
//=============================================================================

TEST_F(MeshRouterTest, BackpressureWhenOutputBusy) {
    std::cout << "\n=== Test BackpressureWhenOutputBusy ===" << std::endl;

    auto router = std::make_unique<MeshRouter>(0, 0, 0, 3, 3);
    std::cout << "1. Router created" << std::endl;

    // Регистрируем LOCAL порты и сохраняем в ports
    auto local_in = std::make_unique<Port>();
    auto local_out = std::make_unique<Port>();
    router->register_in_port(MeshDirection::LOCAL, local_in.get());
    router->register_out_port(MeshDirection::LOCAL, local_out.get());
    ports.push_back(std::move(local_in));
    ports.push_back(std::move(local_out));
    std::cout << "2. LOCAL ports registered" << std::endl;

    // Регистрируем EAST порты
    auto east_in = std::make_unique<Port>();
    auto east_out = std::make_unique<Port>();
    router->register_in_port(MeshDirection::EAST, east_in.get());
    router->register_out_port(MeshDirection::EAST, east_out.get());

    Port* east_out_ptr = east_out.get();
    Port* east_in_ptr = east_in.get();

    ports.push_back(std::move(east_in));
    ports.push_back(std::move(east_out));
    std::cout << "3. EAST ports registered" << std::endl;

    // Проверяем, что порты не nullptr
    ASSERT_NE(router->get_in_port(MeshDirection::LOCAL), nullptr) << "LOCAL in port is nullptr";
    ASSERT_NE(router->get_out_port(MeshDirection::LOCAL), nullptr) << "LOCAL out port is nullptr";
    ASSERT_NE(router->get_in_port(MeshDirection::EAST), nullptr) << "EAST in port is nullptr";
    ASSERT_NE(router->get_out_port(MeshDirection::EAST), nullptr) << "EAST out port is nullptr";
    std::cout << "4. All port pointers are valid" << std::endl;

    // Занимаем EAST выход
    bool sent = east_out_ptr->try_send(Packet(999, 0, 0));
    ASSERT_TRUE(sent) << "Failed to occupy EAST output";
    std::cout << "5. EAST output occupied with packet 999" << std::endl;

    // Проверяем, что EAST выход занят
    ASSERT_TRUE(east_out_ptr->has_data()) << "EAST output should have data";
    std::cout << "6. EAST output has data: " << east_out_ptr->peek()->id << std::endl;

    // Отправляем пакет
    Packet pkt(42, 0, 1);
    bool injected = router->inject_packet(pkt);
    ASSERT_TRUE(injected) << "Packet injection failed";
    std::cout << "7. Packet 42 injected" << std::endl;

    // Проверяем LOCAL вход
    Port* local_in_ptr = router->get_in_port(MeshDirection::LOCAL);
    ASSERT_TRUE(local_in_ptr->has_data()) << "LOCAL input should have packet";
    std::cout << "8. LOCAL input has packet: " << local_in_ptr->peek()->id << std::endl;

    // Обрабатываем такт
    std::cout << "9. Calling router->on_clock()..." << std::endl;
    router->on_clock();
    std::cout << "10. on_clock() completed" << std::endl;

    // Проверяем, что пакет остался на LOCAL входе (backpressure)
    EXPECT_TRUE(local_in_ptr->has_data()) << "Packet should stay in LOCAL input due to backpressure";
    std::cout << "11. LOCAL input still has packet: " << (local_in_ptr->has_data() ? "yes" : "no") << std::endl;

    // Проверяем, что EAST выход все еще занят
    EXPECT_TRUE(east_out_ptr->has_data()) << "EAST output should still be occupied";
    if (east_out_ptr->has_data()) {
        std::cout << "12. EAST output still has packet: " << east_out_ptr->peek()->id << std::endl;
        EXPECT_EQ(east_out_ptr->peek()->id, 999);
    }

    std::cout << "=== Test BackpressureWhenOutputBusy finished ===\n" << std::endl;
}

TEST_F(MeshRouterTest, BackpressureRecovery) {
    std::cout << "\n=== Test BackpressureRecovery ===" << std::endl;

    auto router = std::make_unique<MeshRouter>(0, 0, 0, 3, 3);
    std::cout << "1. Router created" << std::endl;

    // Регистрируем LOCAL порты и сохраняем в ports
    auto local_in = std::make_unique<Port>();
    auto local_out = std::make_unique<Port>();
    router->register_in_port(MeshDirection::LOCAL, local_in.get());
    router->register_out_port(MeshDirection::LOCAL, local_out.get());
    ports.push_back(std::move(local_in));
    ports.push_back(std::move(local_out));
    std::cout << "2. LOCAL ports registered" << std::endl;

    // Регистрируем EAST порты
    auto east_in = std::make_unique<Port>();
    auto east_out = std::make_unique<Port>();
    router->register_in_port(MeshDirection::EAST, east_in.get());
    router->register_out_port(MeshDirection::EAST, east_out.get());

    Port* east_out_ptr = east_out.get();
    Port* east_in_ptr = east_in.get();

    ports.push_back(std::move(east_in));
    ports.push_back(std::move(east_out));
    std::cout << "3. EAST ports registered" << std::endl;

    // Проверяем, что порты не nullptr
    ASSERT_NE(router->get_in_port(MeshDirection::LOCAL), nullptr) << "LOCAL in port is nullptr";
    ASSERT_NE(router->get_out_port(MeshDirection::LOCAL), nullptr) << "LOCAL out port is nullptr";
    ASSERT_NE(router->get_in_port(MeshDirection::EAST), nullptr) << "EAST in port is nullptr";
    ASSERT_NE(router->get_out_port(MeshDirection::EAST), nullptr) << "EAST out port is nullptr";
    std::cout << "4. All port pointers are valid" << std::endl;

    // Занимаем EAST выход
    bool sent = east_out_ptr->try_send(Packet(999, 0, 0));
    ASSERT_TRUE(sent) << "Failed to occupy EAST output";
    std::cout << "5. EAST output occupied with packet 999" << std::endl;

    // Отправляем пакет
    Packet pkt(42, 0, 1);
    bool injected = router->inject_packet(pkt);
    ASSERT_TRUE(injected) << "Packet injection failed";
    std::cout << "6. Packet 42 injected" << std::endl;

    Port* local_in_ptr = router->get_in_port(MeshDirection::LOCAL);
    ASSERT_TRUE(local_in_ptr->has_data()) << "LOCAL input should have packet";
    std::cout << "7. LOCAL input has packet: " << local_in_ptr->peek()->id << std::endl;

    // Первый такт - пакет не проходит
    std::cout << "8. First on_clock() - packet should be blocked..." << std::endl;
    router->on_clock();
    std::cout << "9. First on_clock() completed" << std::endl;

    EXPECT_TRUE(local_in_ptr->has_data()) << "Packet should stay in LOCAL input";
    std::cout << "10. LOCAL input still has packet: yes" << std::endl;

    // Освобождаем выход
    std::cout << "11. Freeing EAST output..." << std::endl;
    auto freed = east_out_ptr->try_recv();
    ASSERT_TRUE(freed.has_value()) << "Failed to free EAST output";
    EXPECT_EQ(freed->id, 999);
    std::cout << "12. EAST output freed, received packet " << freed->id << std::endl;

    EXPECT_FALSE(east_out_ptr->has_data()) << "EAST output should be empty now";
    std::cout << "13. EAST output is empty" << std::endl;

    // Второй такт - пакет должен пройти
    std::cout << "14. Second on_clock() - packet should pass..." << std::endl;
    router->on_clock();
    std::cout << "15. Second on_clock() completed" << std::endl;

    // Проверяем, что пакет ушел с LOCAL входа
    EXPECT_FALSE(local_in_ptr->has_data()) << "Packet should leave LOCAL input";
    std::cout << "16. LOCAL input is empty: " << (local_in_ptr->has_data() ? "no" : "yes") << std::endl;

    // Проверяем, что пакет появился на EAST выходе
    EXPECT_TRUE(east_out_ptr->has_data()) << "Packet should be on EAST output";
    if (east_out_ptr->has_data()) {
        auto peeked = east_out_ptr->peek();
        ASSERT_TRUE(peeked.has_value());
        std::cout << "17. EAST output has packet: " << peeked->id << std::endl;
        EXPECT_EQ(peeked->id, 42);
    }

    std::cout << "=== Test BackpressureRecovery finished ===\n" << std::endl;
}
