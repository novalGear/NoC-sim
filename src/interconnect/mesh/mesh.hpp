/**
 * @file mesh.hpp
 * @brief Реализация Mesh топологии интерконнекта.
 * @author Novoselov Alexander
 * @date 16/03/2026
 */

#pragma once

#include "mesh_utils.hpp"
#include "mesh_router.hpp"
#include "../core/interconnect.hpp"
#include <vector>
#include <memory>
#include <cassert>

class MeshInterconnectTest;

/**
 * @class MeshInterconnect
 * @brief Интерконнект с топологией двумерной сетки (Mesh).
 *
 * @details
 * Реализует Mesh топологию размером width x height:
 * - Каждый узел имеет координаты (x, y)
 * - Используется X-Y маршрутизация (Dimension Order Routing)
 * - Граничные узлы имеют меньше физических соединений
 * - Все порты создаются и управляются этим классом
 *
 * Нумерация узлов: nodeId = y * width + x
 */
class MeshInterconnect final : public Interconnect<MeshRouter> {
private:
    friend class MeshInterconnectTest;

    int width_;   ///< Ширина сетки (количество столбцов)
    int height_;  ///< Высота сетки (количество строк)
    int total_nodes_;
    /// Все роутеры сети (владение)
    // std::vector<std::unique_ptr<MeshRouter>> routers_;

    /// Все порты сети (владение)
    std::vector<std::unique_ptr<Port>> all_ports_;

    /**
     * @brief Получить MeshRouter по координатам.
     * @param[in] x Координата X
     * @param[in] y Координата Y
     * @return Указатель на MeshRouter или nullptr
     */
    [[nodiscard]] MeshRouter* get_router(int x, int y) const;

    /**
     * @brief Получить MeshRouter по индексу.
     * @param[in] node_idx индекс роутера
     * @return Указатель на MeshRouter
     */
    [[nodiscard]] MeshRouter* get_router(size_t node_idx) const;

    /**
     * @brief Создать порт и сохранить владение в all_ports_
     * @return Сырой указатель на созданный порт для регистрации в router
     */
    [[nodiscard]] Port* create_port();

    /**
     * @brief создать все роутеры, сохраняя владение в routers
     */
    void init_routers();

    /**
     * @brief зарегистрировать локальный порт у router
     * @param node_idx индекс router
     */
    void reg_local_ports(int node_idx);

    /**
     * @brief Соединить все роутеры портами
     * @details Соединяет каждый роутер с соседями NORTH и EAST
     */
    void link_all_routers();

    /**
     * @brief Соединить роутер с соседним по выбранному направлению
     * @param[in] node_idx: индекс соединяемого роутера
     * @param[in] dir_1_to_2: направление на соседа (1->2)
     */
    void link_routers(size_t node_idx, MeshDirection dir_1_to_2);

public:
    /**
     * @brief Конструктор MeshInterconnect.
     * @param[in] w Ширина сетки (> 0)
     * @param[in] h Высота сетки (> 0)
     * @param[in] routing_algo Алгоритм маршрутизации (по умолчанию "DOR")
     *
     * @pre w > 0 && h > 0
     */
    MeshInterconnect(int w, int h, const std::string& routing_algo = "DOR")
        : Interconnect(w * h, routing_algo)
        , width_(w)
        , height_(h)
        , total_nodes_(w * h)
    {
        assert(w > 0 && "Width must be positive");
        assert(h > 0 && "Height must be positive");
    }

    /**
     * @brief Деструктор по умолчанию (unique_ptr все очистит).
     */
    ~MeshInterconnect() override = default;

    // Запрет копирования (из-за unique_ptr)
    MeshInterconnect(const MeshInterconnect&) = delete;
    MeshInterconnect& operator=(const MeshInterconnect&) = delete;

    // Разрешение перемещения
    MeshInterconnect(MeshInterconnect&&) noexcept = default;
    MeshInterconnect& operator=(MeshInterconnect&&) noexcept = default;

    /**
     * @brief Построение Mesh топологии.
     * @details
     * 1. Создает все MeshRouter'ы
     * 2. Создает и регистрирует порты для каждого направления
     * 3. Соединяет соседние роутеры парой портов
     */
    void build() override;

    /**
     * @brief Выполнить один такт симуляции.
     * @details Вызывает on_clock() для всех роутеров в сетке.
     */
    void on_clock() override {
        for (auto& router : routers_) {
            router->on_clock();
        }
    }

    /**
     * @brief Отправить пакет в сеть.
     * @param[in] srcNodeId ID узла-источника
     * @param[in] pkt Пакет для отправки
     * @return true если пакет принят в локальный порт
     */
    bool inject_packet(int src_node_idx, const Packet& pkt);

    /**
     * @brief Извлечь пакет из сети.
     * @param[in] dstNodeId ID узла-назначения
     * @return true если пакет извлечен
     */
    std::optional<Packet> eject_packet(int dst_node_idx);

    // Геттеры
    [[nodiscard]] int get_width() const { return width_; }
    [[nodiscard]] int get_height() const { return height_; }
};
