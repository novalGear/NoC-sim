/**
 * @file mesh_router_test.cpp
 * @brief Google Test для MeshRouter (через публичный интерфейс).
 * @author Novoselov Alexander
 * @date 18/03/2026
 */

#include <gtest/gtest.h>
#include "mesh_router.hpp"
#include "mesh_utils.hpp"

class MeshRouterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создаем роутеры через публичный конструктор
        router_center = std::make_unique<MeshRouter>(4, 1, 1, 3, 3);
        router_corner = std::make_unique<MeshRouter>(0, 0, 0, 3, 3);
        router_edge = std::make_unique<MeshRouter>(1, 1, 0, 3, 3);
        router_linear = std::make_unique<MeshRouter>(2, 2, 0, 5, 1);
    }

    std::unique_ptr<MeshRouter> router_center;
    std::unique_ptr<MeshRouter> router_corner;
    std::unique_ptr<MeshRouter> router_edge;
    std::unique_ptr<MeshRouter> router_linear;
};

//=============================================================================
// Тесты инициализации (публичные геттеры)
//=============================================================================

TEST_F(MeshRouterTest, ConstructorInitializes) {
    EXPECT_EQ(router_center->get_id(), 4);
    EXPECT_EQ(router_center->get_coords().x, 1);
    EXPECT_EQ(router_center->get_coords().y, 1);
    EXPECT_EQ(router_center->get_input_count(), 5);
    EXPECT_EQ(router_center->get_output_count(), 5);
}

TEST_F(MeshRouterTest, ConstructorLinearMesh) {
    EXPECT_EQ(router_linear->get_coords().x, 2);
    EXPECT_EQ(router_linear->get_coords().y, 0);
}

//=============================================================================
// Тесты маршрутизации через инжект/извлечение (требуется полная Mesh)
// Так как MeshRouter требует зарегистрированных портов для работы,
// тестировать его в изоляции сложно. Вместо этого используем MeshInterconnect.
//=============================================================================

// Простой тест: проверяем, что роутер можно создать и он имеет правильные параметры
TEST_F(MeshRouterTest, RouterHasCorrectPortCounts) {
    EXPECT_EQ(router_center->get_input_count(), MESH_PORTS_PER_ROUTER);
    EXPECT_EQ(router_center->get_output_count(), MESH_PORTS_PER_ROUTER);
}

TEST_F(MeshRouterTest, RouterHasCorrectCoordinates) {
    EXPECT_EQ(router_center->get_coords().x, 1);
    EXPECT_EQ(router_center->get_coords().y, 1);

    EXPECT_EQ(router_corner->get_coords().x, 0);
    EXPECT_EQ(router_corner->get_coords().y, 0);

    EXPECT_EQ(router_edge->get_coords().x, 1);
    EXPECT_EQ(router_edge->get_coords().y, 0);
}
