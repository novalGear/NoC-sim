/**
 * @file mesh_utils.hpp
 * @brief Вспомогательные утилиты для Mesh топологии интерконнекта.
 * @author Novoselov Alexander
 * @date 16/03/2026
 *
 * @details
 * Этот файл предоставляет общие типы, константы и функции для работы с
 * двумерной Mesh топологией. Содержит:
 * - Перечисление направлений портов Mesh роутера
 * - Константы для количества портов
 * - Структуру координат и функции преобразования
 * - Утилиты для работы с соседними узлами
 */

#pragma once

#include <cassert>

//=============================================================================
// Enum направлений соединений
//=============================================================================

/**
 * @enum MeshDirection
 * @brief Перечисление возможных направлений портов Mesh роутера.
 *
 * @details
 * Каждый роутер в Mesh сетке имеет 5 направлений:
 * - Четыре для соединения с соседними роутерами (север, восток, юг, запад)
 * - NORTH: на увеличение y
 * - EAST:  на увеличение x
 * - Один локальный порт для подключения вычислительного узла (IP core)
 *
 * Значения перечисления используются как индексы портов в массивах.
 */
enum class MeshDirection {
    NORTH = 0,  ///< Северное направление (вверх по оси Y)
    EAST  = 1,  ///< Восточное направление (вправо по оси X)
    SOUTH = 2,  ///< Южное направление (вниз по оси Y)
    WEST  = 3,  ///< Западное направление (влево по оси X)
    LOCAL = 4,  ///< Локальный порт для подключения вычислительного узла
    COUNT       ///< Общее количество направлений (используется для размеров массивов)
};

/**
 * @brief Количество портов на один Mesh роутер.
 * @details Каждый роутер имеет 5 дуплексных портов (вход + выход для каждого направления).
 */
constexpr int MESH_PORTS_PER_ROUTER = static_cast<int>(MeshDirection::COUNT);

/**
 * @brief маска существования порта
 * @details mask[k] - существует (1) или нет (0) k-ый порт,
 * обычно используется через направления, а не индексы в массиве портов
 */
using MeshPortMask = std::bitset<static_cast<size_t>(MeshDirection::COUNT)>;

//=============================================================================
// Координаты
//=============================================================================

/**
 * @struct MeshCoords
 * @brief Представляет двумерные координаты узла в Mesh сетке.
 */
struct MeshCoords {
    int x;  ///< Координата по оси X (столбец), от 0 до width-1
    int y;  ///< Координата по оси Y (строка), от 0 до height-1

    /**
     * @brief Оператор сравнения для координат.
     * @param other Другие координаты для сравнения
     * @return true если координаты равны
     */
    bool operator==(const MeshCoords& other) const {
        return x == other.x && y == other.y;
    }

    /**
     * @brief Проверяет, находятся ли координаты в пределах сетки.
     * @param width Ширина сетки (количество столбцов)
     * @param height Высота сетки (количество строк)
     * @return true если координаты валидны
     */
    bool isValid(int width, int height) const {
        return x >= 0 && x < width && y >= 0 && y < height;
    }
};

/**
 * @brief Преобразует линейный ID узла в двумерные координаты.
 * @param nodeId Линейный идентификатор узла (0 .. width*height-1)
 * @param width Ширина сетки (количество столбцов)
 * @return MeshCoords соответствующие nodeId
 *
 * @details
 * Используется формула: x = nodeId % width, y = nodeId / width
 *
 * @pre width > 0
 * @note Корректность nodeId предполагается (должен быть < width*height)
 */
static inline MeshCoords id2coords(int nodeId, int width) {
    assert(width > 0 && "Width must be positive");
    return {nodeId % width, nodeId / width};
}

/**
 * @brief Преобразует двумерные координаты в линейный ID узла.
 * @param x Координата X (столбец)
 * @param y Координата Y (строка)
 * @param width Ширина сетки (количество столбцов)
 * @return Линейный идентификатор узла
 *
 * @details
 * Используется формула: nodeId = y * width + x
 *
 * @pre x >= 0, y >= 0, width > 0
 */
static inline int coords2id(int x, int y, int width) {
    assert(x >= 0 && y >= 0 && "Coordinates must be non-negative");
    assert(width > 0 && "Width must be positive");
    return y * width + x;
}

/**
 * @brief Преобразует структуру координат в линейный ID узла.
 * @param coords Структура с координатами
 * @param width Ширина сетки (количество столбцов)
 * @return Линейный идентификатор узла
 * @overload
 */
static inline int coords2id(const MeshCoords& coords, int width) {
    return coords2id(coords.x, coords.y, width);
}

/**
 * @brief Проверяет существование соседа у узла в заданном направлении.
 * @param nodeId ID узла
 * @param width Ширина сетки
 * @param height Высота сетки
 * @param dir Направление для проверки
 * @return true если сосед существует (не за границей сетки)
 *
 * @details
 * Для LOCAL направления всегда возвращает true, так как локальный порт
 * считается существующим у всех роутеров.
 */
static inline bool hasNeighbor(int nodeId, int width, int height, MeshDirection dir) {
    if (dir == MeshDirection::LOCAL) return true;  // LOCAL всегда есть

    auto [x, y] = id2coords(nodeId, width);

    switch (dir) {
        case MeshDirection::NORTH: return y > 0;
        case MeshDirection::SOUTH: return y < height - 1;
        case MeshDirection::EAST:  return x < width - 1;
        case MeshDirection::WEST:  return x > 0;
        default: return false;
    }
}

/**
 * @brief Получает ID соседнего узла в заданном направлении.
 * @param nodeId ID исходного узла
 * @param width Ширина сетки
 * @param height Высота сетки
 * @param dir Направление к соседу
 * @return ID соседнего узла или -1 если соседа не существует
 *
 * @details
 * Для LOCAL направления возвращает ID самого узла (локальный "сосед").
 * Если соседа нет (узел на границе), возвращает -1.
 *
 * @see hasNeighbor()
 */
static inline int getNeighborId(int nodeId, int width, int height, MeshDirection dir) {
    if (!hasNeighbor(nodeId, width, height, dir)) {
        return -1;
    }
    if (dir == MeshDirection::LOCAL) return nodeId;  // LOCAL "сосед" - это сам узел

    auto [x, y] = id2coords(nodeId, width);

    switch (dir) {
        case MeshDirection::NORTH: return coords2id(x, y - 1, width);
        case MeshDirection::SOUTH: return coords2id(x, y + 1, width);
        case MeshDirection::EAST:  return coords2id(x + 1, y, width);
        case MeshDirection::WEST:  return coords2id(x - 1, y, width);
        default: return -1;
    }
}

/**
 * @brief Получает противоположное направление.
 * @param dir Исходное направление
 * @return Противоположное направление
 *
 * @details
 * NORTH <-> SOUTH
 * EAST  <-> WEST
 * LOCAL остается LOCAL (нет противоположного)
 */
static inline MeshDirection oppositeDirection(MeshDirection dir) {
    switch (dir) {
        case MeshDirection::NORTH: return MeshDirection::SOUTH;
        case MeshDirection::SOUTH: return MeshDirection::NORTH;
        case MeshDirection::EAST:  return MeshDirection::WEST;
        case MeshDirection::WEST:  return MeshDirection::EAST;
        default: return MeshDirection::LOCAL;
    }
}
