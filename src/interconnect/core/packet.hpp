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
    int id;
    int src;
    int dst;

    // Метрики
    int send_tick = -1;      // такт, когда пакет был инжектирован в сеть
    int recv_tick = -1;      // такт, когда пакет был извлечен из сети
    int hops = 0;           // количество совершенных хопов

    // Полезная нагрузка (для моделирования трафика)
    int size;           // размер в байтах или флитах
    // Или generic: std::vector<uint8_t> payload;

    // Тип трафика (для QoS)
    enum class TrafficType { REQUEST, RESPONSE, BULK_DATA };
    TrafficType type;

    Packet() : id(0), src(0), dst(0), send_tick(-1), recv_tick(-1), hops(0), size(1) {}

    Packet(int _id, int _src, int _dst)
        : id(_id), src(_src), dst(_dst), send_tick(-1), recv_tick(-1), hops(0), size(1) {}

    Packet(int _id, int _src, int _dst, int _send_tick)
        : id(_id), src(_src), dst(_dst), send_tick(_send_tick), recv_tick(-1), hops(0), size(1) {}

    Packet(int _id, int _src, int _dst, int _send_tick, int _size)
        : id(_id), src(_src), dst(_dst), send_tick(_send_tick), recv_tick(-1), hops(0), size(_size) {}

    Packet(int _id, int _src, int _dst, int _send_tick, int _size, TrafficType _type)
        : id(_id), src(_src), dst(_dst), send_tick(_send_tick), recv_tick(-1), hops(0), size(_size), type(_type) {}

    int latency() const {
        if (send_tick >= 0 && recv_tick >= 0) {
            return recv_tick - send_tick;
        }
        return -1;
    }
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
