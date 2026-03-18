/**
 * @file mesh_router.hpp
 * @brief Специализированный маршрутизатор для Mesh топологии.
 * @author Novoselov Alexander
 * @date 16/03/2026
 */

#pragma once

#include <bitset>

#include "router.hpp"
#include "mesh_utils.hpp"
#include <unordered_map>

/**
 * @class MeshRouter
 * @brief Маршрутизатор для двумерной Mesh топологии с X-Y маршрутизацией.
 *
 * @details
 * MeshRouter расширяет базовый класс Router, добавляя:
 * - Хранение координат в сетке
 * - X-Y (dimension-order) маршрутизацию
 * - Регистрацию портов по направлениям MeshDirection
 *
 * Количество и типы портов зависят от позиции роутера в сетке
 * и определяются внешним кодом (MeshInterconnect) при построении топологии.
 */
class MeshRouter final : public Router {
public:
    /**
     * @brief Конструктор MeshRouter.
     * @param[in] id Уникальный идентификатор роутера
     * @param[in] x Координата X в сетке
     * @param[in] y Координата Y в сетке
     * @param[in] width  Ширина всей mesh сетки
     * @param[in] height Высота всей mesh сетки
     *
     * @note Порты не создаются в конструкторе - они будут добавлены позже
     *       через registerInputPort/registerOutputPort.
     */
    MeshRouter(int id, int x, int y, int width, int height);

    /**
     * @brief Получить координаты роутера.
     * @see MeshCoords
     */
    [[nodiscard]] MeshCoords get_coords() const { return coords; }

    /**
     * @brief Проверить наличие входного порта в заданном направлении.
     * @param[in] dir Направление
     * @return true если порт существует
     */
    [[nodiscard]] inline bool has_in_port(MeshDirection dir) const {
        return in_ports_mask.test(static_cast<size_t>(dir));
    }

    /**
     * @brief Проверить наличие выходного порта в заданном направлении.
     * @param[in] dir Направление
     * @return true если порт существует
     */
    [[nodiscard]] inline bool has_out_port(MeshDirection dir) const {
        return out_ports_mask.test(static_cast<size_t>(dir));
    }

    /**
     * @brief Отправить пакет в сеть.
     * @param[in] pkt Пакет для отправки
     * @return true если пакет принят в локальный порт
     */
    bool inject_packet(const& Packet pkt);

    /**
     * @brief Извлечь пакет из сети.
     * @param[out] out_pkt Ссылка для сохранения пакета
     * @return true если пакет извлечен
     */
    bool eject_packet(const& Packet out_pkt);

private:
    MeshCoords coords;           ///< Координаты в сетке
    int grid_width;              ///< Ширина всей сетки
    int grid_height;             ///< Высота всей сетки

    MeshPortMask in_ports_mask;    ///< Маска существования входных портов
    MeshPortMask out_ports_mask;   ///< Маска существования выходных портов

    // Дружественный класс для доступа к регистрации портов
    friend class MeshInterconnect;

    /**
     * @brief Реализация X-Y маршрутизации.
     * @param[in] pkt Пакет для маршрутизации
     * @return Индекс выходного порта
     *
     * @details
     * Алгоритм:
     * 1. Если пакет достиг цели (dst совпадает с координатами) -> LOCAL
     * 2. Иначе если x_dst != x_current -> двигаемся по X (EAST/WEST)
     * 3. Иначе двигаемся по Y (SOUTH/NORTH)
     *
     * @throws std::runtime_error если порт для выбранного направления не существует
     */
    [[nodiscard]] int route_pkt(const Packet& pkt) const override;

    /**
     * @brief Зарегистрировать входной порт (вызывается только MeshInterconnect).
     * @param[in] dir Направление порта
     * @param[in] port - указатель на существующий порт (принадлежит Interconnect)
     *
     * @pre Порт с таким направлением еще не зарегистрирован
     * @pre port* != nullptr
     * @post port* зарегистрирован в input_ports и привязан к dir
     */
    void register_in_port(MeshDirection dir, Port* port);

    /**
     * @brief Зарегистрировать выходной порт (вызывается только MeshInterconnect).
     * @param[in] dir Направление порта
     * @param[in] port - указатель на существующий порт (принадлежит Interconnect)
     *
     * @pre Порт с таким направлением еще не зарегистрирован
     * @pre port* != nullptr
     * @post port* зарегистрирован в output_ports и привязан к dir
     */
    void register_out_port(MeshDirection dir, Port* port);

    /**
     * @brief Получить доступ к входному порту по направлению
     * @param[in] dir Направление порта
     */
    Port* get_in_port(MeshDirection dir);

    /**
     * @brief Получить доступ к выходному порту по направлению
     * @param[in] dir Направление порта
     */
    Port* get_out_port(MeshDirection dir);
};
