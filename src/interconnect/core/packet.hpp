/**
 * @file packet.hpp
 * @brief Базовые структуры данных для симуляции: Packet и Request.
 * @author Novoselov Alexander
 * @date   15/03/2026
 */

#pragma once

#include <vector>

/**
 * @struct Packet
 * @brief Представляет пакет данных, передаваемый через интерконнект.
 */
struct Packet {
    int id;             ///< Уникальный идентификатор пакета
    int src;            ///< ID узла-источника
    int dst;            ///< ID узла-назначения
    int current_hop;    ///< Счетчик хопов (для статистики латентности)

    Packet() : id(0), src(0), dst(0), current_hop(0) {}
    Packet(int _id, int _src, int _dst)
        : id(_id), src(_src), dst(_dst), current_hop(0) {}
};

/**
 * @struct Request
 * @brief Запрос на арбитраж от входного порта.
 * @details Используется классом RRArbiter для разрешения конфликтов.
 */
struct Request {
    int src;    ///< Индекс входного порта, отправившего запрос
    // dest не хранится, смотри AllRequests
    // int dst;    ///< Индекс выходного порта, куда направляется пакет
    Request() : src(-1) {}
    explicit Request(int src_idx) : src(src_idx)  {}
};

// Тип для списка запросов на один выходной порт
using RequestsList = std::vector<Request>;

// Тип для карты всех запросов:
// индекс выхода -> список запросов на этот выход
using AllRequests = std::vector<RequestsList>;
