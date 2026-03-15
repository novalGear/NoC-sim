/**
 * @file Interconnect.hpp
 * @brief Абстрактный базовый класс для моделирования интерконнекта (сети-on-chip).
 * @author Novoselov Alexander
 * @date 15/03/2026
 */

#pragma once

#include "router.hpp"
#include "packet.hpp"

#include <vector>
#include <memory>
#include <string>
#include <cassert>

/**
 * @struct SimStats
 * @brief Структура для хранения статистики работы симуляции.
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
 *
 * @details
 * Этот класс реализует шаблонный метод (Template Method Pattern) для управления симуляцией.
 * Он владеет коллекцией всех маршрутизаторов сети и гарантирует их корректное уничтожение.
 *
 * Наследники (например, MeshInterconnect, ButterflyInterconnect) обязаны реализовать:
 * - Логику создания и настройки маршрутизаторов (метод build()).
 * - Логику одного такта симуляции (метод on_clock()).
 * - Механизмы инжекта и извлечения пакетов.
 *
 * Базовый класс предоставляет:
 * - Управление памятью (std::vector<std::unique_ptr<Router>>).
 * - Универсальный доступ к роутерам по ID (обычно наследникам удобно
 *   определить функции перевода ID в координаты соответственно топологии).
 * - Общий цикл симуляции (метод run()).
 *
 * @note Класс не поддерживает копирование (copy constructor/assignment deleted),
 *       так как владеет уникальными указателями. Поддерживает перемещение (move).
 *
 * @see Router, MeshInterconnect, ButterflyInterconnect
 */
class Interconnect {
protected:
    // === Данные, общие для всех топологий ===

    int total_nodes;                      ///< Общее количество узлов в сети
    std::string routing_algo;             ///< Используемый алгоритм маршрутизации

    /**
     * @brief Контейнер всех маршрутизаторов сети.
     * @details Владеет объектами роутеров. Гарантирует их удаление при разрушении сети.
     *          Наследники заполняют этот вектор в методе build().
     */
    std::vector<std::unique_ptr<Router>> routers;

    /**
     * @brief Защищенный конструктор для инициализации общих полей.
     * @param[in] nodes Общее количество узлов.
     * @param[in] routing Название алгоритма маршрутизации.
     *
     * @pre nodes > 0
     */
    explicit Interconnect(int nodes, const std::string& routing)
        : total_nodes(nodes), routing_algo(routing) {
        assert(nodes > 0 && "Total nodes must be positive");
    }

public:
    // === Жизненный цикл ===

    /**
     * @brief Виртуальный деструктор.
     * @details Обеспечивает корректное удаление объектов наследников через указатель на базу.
     */
    virtual ~Interconnect() = default;

    // Запрет копирования
    Interconnect(const Interconnect&) = delete;
    Interconnect& operator=(const Interconnect&) = delete;

    // Разрешение перемещения (по умолчанию работает корректно для unique_ptr)
    Interconnect(Interconnect&&) noexcept = default;
    Interconnect& operator=(Interconnect&&) noexcept = default;

    // === Обязательный интерфейс (Pure Virtual Interface) ===

    /**
     * @brief Построение топологии сети.
     * @details
     * Наследник должен:
     * 1. Очистить и заполнить вектор #routers нужными объектами роутеров.
     * 2. Установить соединения между портами роутеров (связать выходы одних с входами других).
     * 3. Инициализировать внутренние структуры данных, специфичные для топологии.
     *
     * @note Вызывается автоматически перед началом симуляции в методе run().
     * @note Не должен вызываться повторно без предварительного сброса состояния.
     * @see Router
     */
    virtual void build() = 0;

    /**
     * @brief Выполнение одного такта симуляции.
     * @details
     * Наследник должен реализовать логику продвижения времени на один такт:
     * - Обновление состояний всех роутеров (вызов Router::on_clock()).
     * - Обновление каналов связи (если используются отдельные объекты каналов - port).
     * - Сбор локальной статистики (опционально).
     *
     * @note Порядок обхода роутеров может влиять на детерминизм симуляции.
     * @see Router, Port
     */
    virtual void on_clock() = 0;

    /**
     * @brief Инжект пакета в сеть от имени узла-источника.
     * @param[in] srcNodeId Универсальный ID узла-источника [0, total_nodes).
     * @param[in] pkt Пакет для отправки.
     * @return true если пакет успешно принят в буфер роутера, false иначе (например, буфер занят).
     *
     * @details
     * Наследник должен определить, какому физическому роутеру принадлежит srcNodeId,
     * и попытаться отправить пакет в его локальный входной порт.
     * Реализует механизм backpressure на границе сети.
     * @see Router, Port
     */
    virtual bool injectPacket(int srcNodeId, const Packet& pkt) = 0;

    /**
     * @brief Извлечение доставленного пакета из сети.
     * @param[in] dstNodeId Универсальный ID узла-назначения.
     * @param[out] outPkt Ссылка на переменную для сохранения пакета.
     * @return true если пакет был извлечен, false если выходной пуст.
     *
     * @details
     * Наследник должен проверить локальный выходной порт соответствующего роутера.
     * Обычно вызывается после on_clock() для проверки прибытия пакетов.
     * @see Router, Port
     */
    virtual bool ejectPacket(int dstNodeId, Packet& outPkt) = 0;

    /**
     * @brief Получение указателя на роутер по его универсальному ID.
     * @param[in] nodeId ID узла [0, total_nodes).
     * @return Указатель на Router или nullptr, если ID некорректен.
     *
     * @details
     * Реализация по умолчанию в базовом классе использует прямой доступ к вектору #routers.
     * Предполагается, что ID узла соответствует индексу в векторе routers.
     * Если наследник использует другую схему маппинга, он должен переопределить этот метод.
     * @see Router
     */
    virtual Router* getRouter(int nodeId) {
        if (nodeId < 0 || nodeId >= total_nodes) return nullptr;
        // Безопасный доступ, так как размер вектора должен совпадать с total_nodes
        if (nodeId >= static_cast<int>(routers.size())) return nullptr;
        return routers[nodeId].get();
    }

    // === Шаблонные методы (Template Methods) ===

    /**
     * @brief Запуск полной симуляции на заданное количество тактов.
     * @param[in] cycles Количество тактов для симуляции.
     * @return Структура SimStats с накопленной статистикой.
     *
     * @details
     * Стандартный алгоритм выполнения:
     * 1. Вызывает build() для инициализации топологии.
     * 2. В цикле вызывает on_clock() заданное число раз.
     * 3. (Опционально) Вызывает сбор финальной статистики.
     *
     * @note Наследники могут переопределить этот метод, если требуется нестандартный цикл,
     *       но рекомендуется использовать базовую реализацию и переопределять on_clock()/build().
     */
    virtual SimStats run(int cycles) {
        build();
        SimStats stats;
        stats.total_cycles = cycles;

        for (int i = 0; i < cycles; ++i) {
            on_clock();
            // Здесь можно добавить вызов виртуального метода collectStepStats(stats);
        }

        return stats;
    }

    /**
     * @brief Получить общее количество узлов в сети.
     * @return int Количество узлов.
     */
    [[nodiscard]] int getTotalNodes() const { return total_nodes; }

    /**
     * @brief Получить название используемого алгоритма маршрутизации.
     * @return std::string Название алгоритма.
     */
    [[nodiscard]] const std::string& getRoutingAlgo() const { return routing_algo; }
};
