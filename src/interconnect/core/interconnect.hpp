/**
@file interconnect.hpp
@brief Абстрактный базовый класс для моделирования интерконнекта (сети-on-chip).
@author Novoselov Alexander
@date 15/03/2026
*/
#pragma once
#include "router.hpp"
#include "packet.hpp"
#include <vector>
#include <memory>
#include <string>
#include <cassert>
#include <optional>

class MeshInterconnectTest;

/**
@struct SimStats
@brief Структура для хранения статистики работы симуляции.
*/
struct SimStats {
    double avg_latency = 0.0;   ///< Средняя задержка доставки пакетов (в тактах)
    double max_latency = 0.0;   ///< Максимальная задержка
    int packets_sent = 0;       ///< Всего отправлено пакетов
    int packets_delivered = 0;  ///< Успешно доставлено
    int packets_lost = 0;       ///< Потеряно (если реализована логика потерь)
    long long total_cycles = 0; ///< Количество тактов симуляции
};

/**
 * @class Interconnect
 * @brief Абстрактный базовый класс (ABC), определяющий интерфейс сети интерконнекта.
 * @details
 * Этот класс реализует шаблонный метод (Template Method Pattern) для управления симуляцией.
 * Он владеет коллекцией всех маршрутизаторов сети и гарантирует их корректное уничтожение.
 * Наследники (например, MeshInterconnect, ButterflyInterconnect) обязаны реализовать:
 * Логику создания и настройки маршрутизаторов (метод build()).
 * Логику одного такта симуляции (метод on_clock()).
 * Механизмы инжекта и извлечения пакетов.
 * Базовый класс предоставляет:
 * Управление памятью (std::vector<std::unique_ptr>).
 * Универсальный доступ к роутерам по ID.
 * Общий цикл симуляции (метод run()).
 * @note Класс не поддерживает копирование. Поддерживает перемещение.
 * @see Router, MeshInterconnect, ButterflyInterconnect
*/
template <typename ArbiterT = RRArbiter>
class Interconnect {
    friend class MeshInterconnectTest;

protected:
    // === Данные, общие для всех топологий ===
    int total_nodes;                      ///< Общее количество узлов в сети
    std::string routing_algo;             ///< Используемый алгоритм маршрутизации

    /**
    @brief Контейнер всех маршрутизаторов сети.
    @details Владеет объектами роутеров. Гарантирует их удаление при разрушении сети.
         Наследники заполняют этот вектор в методе build().
    */
    std::vector<std::unique_ptr<Router<ArbiterT>>> routers_;

    /// Все порты сети (владение)
    std::vector<std::unique_ptr<Port>> all_ports_;

    /**
    @brief Защищенный конструктор для инициализации общих полей.
    @param[in] nodes Общее количество узлов.
    @param[in] routing Название алгоритма маршрутизации.
    @pre nodes > 0
    */
    explicit Interconnect(int nodes, const std::string& routing);

public:
    // === Жизненный цикл ===
    virtual ~Interconnect() = default;

    // Запрет копирования
    Interconnect(const Interconnect &) = delete;
    Interconnect & operator=(const Interconnect &) = delete;

    // Разрешение перемещения
    Interconnect(Interconnect &&) noexcept = default;
    Interconnect & operator=(Interconnect &&) noexcept = default;

    // === Обязательный интерфейс (Pure Virtual Interface) ===

    /**
     * @brief Построение топологии сети.
     * @details
     * Наследник должен:
     * 1. Очистить и заполнить вектор #routers нужными объектами роутеров.
     * 2. Установить соединения между портами роутеров.
     * 3. Инициализировать внутренние структуры данных.
     */
    virtual void build() = 0;

    /**
     * @brief Выполнение одного такта симуляции.
     * @details
     * Наследник должен реализовать логику продвижения времени на один такт:
     * - Обновление состояний всех роутеров.
     * - Обновление каналов связи.
     */
    virtual void on_clock() = 0;

    /**
     * @brief Инжект пакета в сеть от имени узла-источника.
     * @param[in] srcNodeId Универсальный ID узла-источника [0, total_nodes).
     * @param[in] pkt Пакет для отправки.
     * @return true если пакет успешно принят в буфер роутера, false иначе.
     */
    virtual bool inject_packet(int srcNodeId, const Packet & pkt) = 0;

    /**
     * @brief Извлечение доставленного пакета из сети.
     * @param[in] dstNodeId Универсальный ID узла-назначения.
     * @return Packet если пакет был извлечен, std::nullopt если выходной пуст.
     */
    virtual std::optional<Packet> eject_packet(int dstNodeId) = 0;

    /**
     * @brief Получение указателя на роутер по его универсальному ID.
     * @param[in] nodeId ID узла [0, total_nodes).
     * @return Указатель на Router или nullptr, если ID некорректен.
     */
    virtual Router<ArbiterT>* get_router(int nodeId);

    // === Шаблонные методы (Template Methods) ===

    /**
     * @brief Запуск полной симуляции на заданное количество тактов.
     * @param[in] cycles Количество тактов для симуляции.
     * @return Структура SimStats с накопленной статистикой.
     */
    virtual SimStats run(int cycles);

    /**
     * @brief Получить общее количество узлов в сети.
     * @return int Количество узлов.
     */
    [[nodiscard]] int get_total_nodes() const { return total_nodes; }

    /**
     * @brief Получить название используемого алгоритма маршрутизации.
     * @return std::string Название алгоритма.
     */
    [[nodiscard]] const std::string & get_routing_algo() const { return routing_algo; }
};

// Include template implementations
#include "interconnect.tpp"
