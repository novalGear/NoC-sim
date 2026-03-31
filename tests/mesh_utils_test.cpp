/**
 * @file mesh_utils_test.cpp
 * @brief Google Test для утилитных функций Mesh топологии.
 * @author Novoselov Alexander
 * @date 18/03/2026
 */

#include <gtest/gtest.h>
#include <iostream>
#include "mesh_utils.hpp"

//=============================================================================
// Тесты преобразования координат
//=============================================================================

TEST(MeshUtilsTest, Id2Coords) {
    auto coords = id2coords(5, 3);
    EXPECT_EQ(coords.x, 2);
    EXPECT_EQ(coords.y, 1);

    coords = id2coords(0, 3);
    EXPECT_EQ(coords.x, 0);
    EXPECT_EQ(coords.y, 0);

    coords = id2coords(8, 3);
    EXPECT_EQ(coords.x, 2);
    EXPECT_EQ(coords.y, 2);
}

TEST(MeshUtilsTest, Coords2Id) {
    int id = coords2id(2, 1, 3, 3);
    EXPECT_EQ(id, 5);

    id = coords2id(1, 1, 3, 3);
    EXPECT_EQ(id, 4);

    id = coords2id(0, 0, 3, 3);
    EXPECT_EQ(id, 0);

    id = coords2id(2, 2, 3, 3);
    EXPECT_EQ(id, 8);

    MeshCoords coords{1, 2};
    id = coords2id(coords, 3, 3);
    EXPECT_EQ(id, 7);
}

TEST(MeshUtilsTest, CoordsIsValid) {
    MeshCoords coords{1, 1};
    EXPECT_TRUE(coords.is_valid(3, 3));

    coords = {0, 0};
    EXPECT_TRUE(coords.is_valid(3, 3));

    coords = {-1, 1};
    EXPECT_FALSE(coords.is_valid(3, 3));

    coords = {1, 3};
    EXPECT_FALSE(coords.is_valid(3, 3));
}

//=============================================================================
// Тесты проверки наличия соседей
//=============================================================================

TEST(MeshUtilsTest, HasNeighbor) {
    // 3x3 сетка (width=3, height=3)
    MeshCoords center{1, 1};
    EXPECT_TRUE(has_neighbor(center, 3, 3, MeshDirection::NORTH));  // y+1=2
    EXPECT_TRUE(has_neighbor(center, 3, 3, MeshDirection::EAST));   // x+1=2
    EXPECT_TRUE(has_neighbor(center, 3, 3, MeshDirection::SOUTH));  // y-1=0
    EXPECT_TRUE(has_neighbor(center, 3, 3, MeshDirection::WEST));   // x-1=0
    EXPECT_TRUE(has_neighbor(center, 3, 3, MeshDirection::LOCAL));

    // Угловой узел (0,0)
    MeshCoords corner{0, 0};
    EXPECT_FALSE(has_neighbor(corner, 3, 3, MeshDirection::SOUTH));  // y-1=-1 - нет
    EXPECT_FALSE(has_neighbor(corner, 3, 3, MeshDirection::WEST));   // x-1=-1 - нет
    EXPECT_TRUE(has_neighbor(corner, 3, 3, MeshDirection::NORTH));   // y+1=1 - есть
    EXPECT_TRUE(has_neighbor(corner, 3, 3, MeshDirection::EAST));    // x+1=1 - есть

    // Угловой узел (2,2)
    MeshCoords opposite_corner{2, 2};
    EXPECT_FALSE(has_neighbor(opposite_corner, 3, 3, MeshDirection::NORTH)); // y+1=3 - нет
    EXPECT_FALSE(has_neighbor(opposite_corner, 3, 3, MeshDirection::EAST));  // x+1=3 - нет
    EXPECT_TRUE(has_neighbor(opposite_corner, 3, 3, MeshDirection::SOUTH));  // y-1=1 - есть
    EXPECT_TRUE(has_neighbor(opposite_corner, 3, 3, MeshDirection::WEST));   // x-1=1 - есть

    // Крайний узел (1,0) - нижняя граница (y=0)
    MeshCoords bottom_edge{1, 0};
    EXPECT_FALSE(has_neighbor(bottom_edge, 3, 3, MeshDirection::SOUTH)); // y-1=-1 - нет
    EXPECT_TRUE(has_neighbor(bottom_edge, 3, 3, MeshDirection::NORTH));  // y+1=1 - есть
    EXPECT_TRUE(has_neighbor(bottom_edge, 3, 3, MeshDirection::EAST));   // x+1=2 - есть
    EXPECT_TRUE(has_neighbor(bottom_edge, 3, 3, MeshDirection::WEST));   // x-1=0 - есть

    // 1x5 сетка: width=1, height=5 (вертикальная)
    // Узел (0,0) - нижняя граница
    MeshCoords bottom{0, 0};
    EXPECT_TRUE(has_neighbor(bottom, 1, 5, MeshDirection::NORTH));   // y+1=1 - есть
    EXPECT_FALSE(has_neighbor(bottom, 1, 5, MeshDirection::SOUTH));  // y-1=-1 - нет
    EXPECT_FALSE(has_neighbor(bottom, 1, 5, MeshDirection::EAST));   // x+1=1 - нет (width=1)
    EXPECT_FALSE(has_neighbor(bottom, 1, 5, MeshDirection::WEST));   // x-1=-1 - нет

    // Узел (0,4) - верхняя граница
    MeshCoords top{0, 4};
    EXPECT_FALSE(has_neighbor(top, 1, 5, MeshDirection::NORTH));  // y+1=5 - нет
    EXPECT_TRUE(has_neighbor(top, 1, 5, MeshDirection::SOUTH));   // y-1=3 - есть

    // Узел (0,2) - середина
    MeshCoords vert_middle{0, 2};
    EXPECT_TRUE(has_neighbor(vert_middle, 1, 5, MeshDirection::NORTH));  // y+1=3 - есть
    EXPECT_TRUE(has_neighbor(vert_middle, 1, 5, MeshDirection::SOUTH));  // y-1=1 - есть

    // 5x1 сетка: width=5, height=1 (горизонтальная)
    // Узел (0,0) - левая граница
    MeshCoords left{0, 0};
    EXPECT_TRUE(has_neighbor(left, 5, 1, MeshDirection::EAST));   // x+1=1 - есть
    EXPECT_FALSE(has_neighbor(left, 5, 1, MeshDirection::WEST));  // x-1=-1 - нет
    EXPECT_FALSE(has_neighbor(left, 5, 1, MeshDirection::NORTH)); // y+1=1 - нет (height=1)
    EXPECT_FALSE(has_neighbor(left, 5, 1, MeshDirection::SOUTH)); // y-1=-1 - нет

    // Узел (4,0) - правая граница
    MeshCoords right{4, 0};
    EXPECT_FALSE(has_neighbor(right, 5, 1, MeshDirection::EAST));  // x+1=5 - нет
    EXPECT_TRUE(has_neighbor(right, 5, 1, MeshDirection::WEST));   // x-1=3 - есть

    // Узел (2,0) - середина
    MeshCoords horiz_middle{2, 0};
    EXPECT_TRUE(has_neighbor(horiz_middle, 5, 1, MeshDirection::EAST));   // x+1=3 - есть
    EXPECT_TRUE(has_neighbor(horiz_middle, 5, 1, MeshDirection::WEST));   // x-1=1 - есть
}

//=============================================================================
// Тесты получения ID соседа
//=============================================================================

TEST(MeshUtilsTest, GetNeighborId) {
    // 3x3 сетка (width=3, height=3)
    MeshCoords center{1, 1};
    // NORTH: (1,2) -> ID = 2*3 + 1 = 7? НЕТ! NORTH это y+1 = 2, x=1 -> ID=2*3+1=7
    // но в старом тесте было 1. Давайте проверим логику:
    // В реализации: NORTH = y + 1
    // Если (1,1) и NORTH, то (1,2) - это ID=2*3+1=7
    // Но в старом тесте ожидалось 1. Значит в старом тесте была ошибка!
    // Правильно: NORTH = увеличение y (вверх по координатам)

    EXPECT_EQ(get_neighbor_id(center, 3, 3, MeshDirection::NORTH), coords2id(1, 2, 3, 3));  // (1,2) -> 7
    EXPECT_EQ(get_neighbor_id(center, 3, 3, MeshDirection::EAST),  coords2id(2, 1, 3, 3));  // (2,1) -> 5
    EXPECT_EQ(get_neighbor_id(center, 3, 3, MeshDirection::SOUTH), coords2id(1, 0, 3, 3));  // (1,0) -> 1
    EXPECT_EQ(get_neighbor_id(center, 3, 3, MeshDirection::WEST),  coords2id(0, 1, 3, 3));  // (0,1) -> 3
    EXPECT_EQ(get_neighbor_id(center, 3, 3, MeshDirection::LOCAL), coords2id(1, 1, 3, 3));  // (1,1) -> 4

    // Угловой узел (0,0)
    MeshCoords corner{0, 0};
    EXPECT_EQ(get_neighbor_id(corner, 3, 3, MeshDirection::WEST), -1);
    EXPECT_EQ(get_neighbor_id(corner, 3, 3, MeshDirection::SOUTH), -1);
    EXPECT_EQ(get_neighbor_id(corner, 3, 3, MeshDirection::NORTH), coords2id(0, 1, 3, 3));  // (0,1) -> 3
    EXPECT_EQ(get_neighbor_id(corner, 3, 3, MeshDirection::EAST),  coords2id(1, 0, 3, 3));  // (1,0) -> 1

    // Угловой узел (2,2)
    MeshCoords opposite_corner{2, 2};
    EXPECT_EQ(get_neighbor_id(opposite_corner, 3, 3, MeshDirection::NORTH), -1);
    EXPECT_EQ(get_neighbor_id(opposite_corner, 3, 3, MeshDirection::EAST), -1);
    EXPECT_EQ(get_neighbor_id(opposite_corner, 3, 3, MeshDirection::SOUTH), coords2id(2, 1, 3, 3));  // (2,1) -> 7
    EXPECT_EQ(get_neighbor_id(opposite_corner, 3, 3, MeshDirection::WEST),  coords2id(1, 2, 3, 3));  // (1,2) -> 5

    // 1x5 сетка: width=1, height=5 (вертикальная)
    // Узел (0,0) - нижняя граница
    MeshCoords bottom{0, 0};
    EXPECT_EQ(get_neighbor_id(bottom, 1, 5, MeshDirection::NORTH), coords2id(0, 1, 1, 5));  // (0,1) -> 1
    EXPECT_EQ(get_neighbor_id(bottom, 1, 5, MeshDirection::SOUTH), -1);
    EXPECT_EQ(get_neighbor_id(bottom, 1, 5, MeshDirection::EAST), -1);
    EXPECT_EQ(get_neighbor_id(bottom, 1, 5, MeshDirection::WEST), -1);

    // Узел (0,4) - верхняя граница
    MeshCoords top{0, 4};
    EXPECT_EQ(get_neighbor_id(top, 1, 5, MeshDirection::NORTH), -1);
    EXPECT_EQ(get_neighbor_id(top, 1, 5, MeshDirection::SOUTH), coords2id(0, 3, 1, 5));  // (0,3) -> 3

    // Узел (0,2) - середина
    MeshCoords vert_middle{0, 2};
    EXPECT_EQ(get_neighbor_id(vert_middle, 1, 5, MeshDirection::NORTH), coords2id(0, 3, 1, 5));  // (0,3) -> 3
    EXPECT_EQ(get_neighbor_id(vert_middle, 1, 5, MeshDirection::SOUTH), coords2id(0, 1, 1, 5));  // (0,1) -> 1
    EXPECT_EQ(get_neighbor_id(vert_middle, 1, 5, MeshDirection::EAST), -1);
    EXPECT_EQ(get_neighbor_id(vert_middle, 1, 5, MeshDirection::WEST), -1);

    // 5x1 сетка: width=5, height=1 (горизонтальная)
    // Узел (0,0) - левая граница
    MeshCoords left{0, 0};
    EXPECT_EQ(get_neighbor_id(left, 5, 1, MeshDirection::EAST),  coords2id(1, 0, 5, 1));  // (1,0) -> 1
    EXPECT_EQ(get_neighbor_id(left, 5, 1, MeshDirection::WEST), -1);
    EXPECT_EQ(get_neighbor_id(left, 5, 1, MeshDirection::NORTH), -1);
    EXPECT_EQ(get_neighbor_id(left, 5, 1, MeshDirection::SOUTH), -1);

    // Узел (4,0) - правая граница
    MeshCoords right{4, 0};
    EXPECT_EQ(get_neighbor_id(right, 5, 1, MeshDirection::EAST), -1);
    EXPECT_EQ(get_neighbor_id(right, 5, 1, MeshDirection::WEST), coords2id(3, 0, 5, 1));  // (3,0) -> 3

    // Узел (2,0) - середина
    MeshCoords horiz_middle{2, 0};
    EXPECT_EQ(get_neighbor_id(horiz_middle, 5, 1, MeshDirection::EAST), coords2id(3, 0, 5, 1));  // (3,0) -> 3
    EXPECT_EQ(get_neighbor_id(horiz_middle, 5, 1, MeshDirection::WEST), coords2id(1, 0, 5, 1));  // (1,0) -> 1
    EXPECT_EQ(get_neighbor_id(horiz_middle, 5, 1, MeshDirection::NORTH), -1);
    EXPECT_EQ(get_neighbor_id(horiz_middle, 5, 1, MeshDirection::SOUTH), -1);
}

//=============================================================================
// Тесты противоположных направлений
//=============================================================================

TEST(MeshUtilsTest, OppositeDirection) {
    EXPECT_EQ(opposite(MeshDirection::NORTH), MeshDirection::SOUTH);
    EXPECT_EQ(opposite(MeshDirection::SOUTH), MeshDirection::NORTH);
    EXPECT_EQ(opposite(MeshDirection::EAST), MeshDirection::WEST);
    EXPECT_EQ(opposite(MeshDirection::WEST), MeshDirection::EAST);
    EXPECT_EQ(opposite(MeshDirection::LOCAL), MeshDirection::LOCAL);
}

//=============================================================================
// Тесты MeshCoords операторов
//=============================================================================

TEST(MeshUtilsTest, MeshCoordsEquality) {
    MeshCoords c1{1, 2};
    MeshCoords c2{1, 2};
    MeshCoords c3{2, 1};

    EXPECT_TRUE(c1 == c2);
    EXPECT_FALSE(c1 == c3);
}
