/**
@file mesh_router.hpp
@brief Специализированный маршрутизатор для Mesh топологии.
@author Novoselov Alexander
@date 16/03/2026
*/
#pragma once
#include "router.hpp"
#include "mesh_utils.hpp"
#include <cassert>
#include <optional>

// Forward declaration for tests if needed
class MeshRouterTest;

/**
 * @class MeshRouter
 * @brief Маршрутизатор для двумерной Mesh топологии с X-Y маршрутизацией.
 * @details
 * MeshRouter расширяет базовый класс Router, добавляя:
 * - Хранение координат в сетке
 * - X-Y (dimension-order) маршрутизацию
 * - Регистрацию портов по направлениям MeshDirection
*/
template <typename ArbiterT = RRArbiter>
class MeshRouter final : public Router<ArbiterT> {
    friend class MeshRouterTest;

private:
    MeshCoords coords;           ///< Координаты в сетке
    int grid_width;              ///< Ширина всей сетки
    int grid_height;             ///< Высота всей сетки

public:
// protected:
    /**
     * @brief Реализация X-Y маршрутизации.
     * @param[in] pkt Пакет для маршрутизации
     * @return Индекс выходного порта
     */
    [[nodiscard]] int route_pkt(const Packet& pkt) const override;

// public:
    /**
    * @brief Конструктор MeshRouter.
    * @param[in] id Уникальный идентификатор роутера
    * @param[in] x Координата X в сетке
    * @param[in] y Координата Y в сетке
    * @param[in] width  Ширина всей mesh сетки
    * @param[in] height Высота всей mesh сетки
    */
    MeshRouter(int id, int x, int y, int width, int height);

    /**
     * @brief Зарегистрировать входной порт (вызывается только MeshInterconnect).
     * @param[in] dir Направление порта
     * @param[in] port - указатель на существующий порт
     */
    void register_in_port(MeshDirection dir, Port* port);

    /**
     * @brief Зарегистрировать выходной порт (вызывается только MeshInterconnect).
     * @param[in] dir Направление порта
     * @param[in] port - указатель на существующий порт
     */
    void register_out_port(MeshDirection dir, Port* port);

    /**
     * @brief Получить доступ к входному порту по направлению
     */
    Port* get_in_port(MeshDirection dir);

    /**
     * @brief Получить доступ к выходному порту по направлению
     */
    Port* get_out_port(MeshDirection dir);

    /**
     * @brief Получить координаты роутера.
     */
    [[nodiscard]] MeshCoords get_coords() const { return coords; }

    /**
     * @brief Проверить наличие входного порта в заданном направлении.
     */
    [[nodiscard]] inline bool has_in_port(MeshDirection dir) const {
        return this->in_ports_mask.test(static_cast<size_t>(dir));
    }

    /**
     * @brief Проверить наличие выходного порта в заданном направлении.
     */
    [[nodiscard]] inline bool has_out_port(MeshDirection dir) const {
        return this->out_ports_mask.test(static_cast<size_t>(dir));
    }

    /**
     * @brief Отправить пакет в сеть.
     */
    bool inject_packet(const Packet& pkt);

    /**
     * @brief Извлечь пакет из сети.
     */
    std::optional<Packet> eject_packet();
};

// Include template implementations
#include "mesh_router.tpp"
