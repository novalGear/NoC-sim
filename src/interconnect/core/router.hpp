/**
@file router.hpp
@brief Абстрактный базовый класс маршрутизатора для симуляции интерконнекта.
@author Novoselov Alexandr
@date   22/04/2026
*/
#pragma once
#include "packet.hpp"
#include "port.hpp"
#include "rr_arbiter.hpp"
#include "debug.hpp"
#include <vector>
#include <bitset>
#include <cassert>
#include <iostream>

class MeshRouterTest;

constexpr size_t MAX_PORTS = 64;
using PortMask = std::bitset<MAX_PORTS>;

/**
 @class Router
 @brief Абстрактный базовый класс маршрутизатора с арбитражем Round-Robin.
 @details
    Router определяет общий интерфейс и реализует цикл-точное поведение
    аппаратного маршрутизатора в сети-on-chip:
    Имеет фиксированное количество входных и выходных портов
    На каждом такте (on_clock) выполняет трехфазный алгоритм:
    Collect: Сбор заявок от входных портов (без извлечения пакетов)
    Arbitrate: Разрешение конфликтов через RRArbiter (Round-Robin)
    Send: Фактическая передача выигравших пакетов на выходные порты
    Наследники (например, MeshRouter, ButterflyRouter) обязаны реализовать:
    Логику маршрутизации пакетов (метод route_pkt())
    Специфичную для топологии инициализацию портов
    Механизм backpressure:
    Пакет извлекается из входного порта только если выходной порт готов (isReady)
    Если выход занят, пакет остается во входном буфере до следующего такта
 @note Класс не поддерживает копирование (delete), но поддерживает перемещение.
 @note Все вызовы должны происходить из одного потока симуляции.
 @see Port, RRArbiter, Packet, Interconnect, MeshRouter, ButterflyRouter
*/
template <typename ArbiterT = RRArbiter>
class Router {
    // friend class MeshRouterTest;

private:
    int id;  ///< Уникальный идентификатор маршрутизатора в сети

protected:
    std::vector<Port*> input_ports;   ///< Входные порты маршрутизатора
    std::vector<Port*> output_ports;  ///< Выходные порты маршрутизатора
    int in_port_count;   ///< Количество входных портов (кэшировано для производительности)
    int out_port_count;  ///< Количество выходных портов (кэшировано для производительности)

    PortMask in_ports_mask;    ///< Маска существования входных портов
    PortMask out_ports_mask;   ///< Маска существования выходных портов

    ArbiterT arbiter;   ///< Арбитр для разрешения конфликтов на выходных портах

    /**
     * @brief Чисто виртуальный метод маршрутизации пакета.
     * @param[in] pkt Пакет для маршрутизации.
     * @return Индекс выходного порта, на который должен быть отправлен пакет.
     *
     * @details
     * Наследники должны реализовать этот метод в соответствии с топологией сети.
     * Алгоритм маршрутизации определяет, через какой выходной порт пакет должен
     * покинуть текущий роутер, чтобы достичь узла назначения pkt.dst.
     *
     * @note Гарантируется, что возвращаемый индекс валиден (0 <= idx < out_port_count).
     * @see MeshRouter::route_pkt(), ButterflyRouter::route_pkt()
     */
    [[nodiscard]] virtual int route_pkt(const Packet& pkt) const = 0;

public:
    /**
    * @brief Конструктор маршрутизатора.
    * @param[in] input_count Количество входных портов (должно быть > 0).
    * @param[in] output_count Количество выходных портов (должно быть > 0).
    * @param[in] router_id Уникальный идентификатор (по умолчанию 0).
    *
    * @pre input_count > 0 && output_count > 0
    * @post Инициализирует векторы портов и арбитр с заданными размерами.
    */
    Router(int input_count, int output_count, int router_id = 0);

    /**
     * @brief Виртуальный деструктор.
     * @details Обеспечивает корректное удаление объектов наследников.
     */
    virtual ~Router() = default;

    // Запрет копирования
    Router(const Router &) = delete;
    Router & operator=(const Router &) = delete;

    // Разрешение перемещения
    Router(Router &&) noexcept = default;
    Router & operator=(Router &&) noexcept = default;

    /**
     * @brief Основной метод симуляции: выполняет один такт работы маршрутизатора.
     *
     * @details
     * Алгоритм выполнения такта (трехфазный):
     * 1. **collect_requests()**: Сканирует входные порты, собирает заявки на отправку.
     *    Пакеты НЕ извлекаются из портов на этом этапе (используется peek).
     * 2. **arbitrate_all()**: Для каждого выходного порта запускает арбитраж среди
     *    запросивших его входных портов. Выбирает победителей по алгоритму Round-Robin.
     * 3. **send_all()**: Фактически передает пакеты от победивших входов к выходам.
     *    Проверяет backpressure: если выход занят, пакет остается на входе.
     *
     * @note Метод должен вызываться синхронно для всех роутеров сети в каждом такте симуляции.
     * @note Изменение состояния портов происходит только в фазе send_all.
     *
     * @see collect_requests(), arbitrate_all(), send_all()
     */
    void on_clock();

    /**
     * @brief Фаза 1: Сбор заявок от входных портов.
     * @param[out] requests Карта запросов, заполняемая методом.
     *
     * @details
     * Для каждого входного порта:
     * - Если порт содержит пакет (hasData), определяется целевой выходной порт через route_pkt()
     * - Индекс входного порта добавляется в список запросов для целевого выхода
     *
     * @pre requests.size() == out_port_count
     * @post requests[out_idx] содержит индексы входов, желающих отправить на out_idx.
     * @note Пакеты НЕ извлекаются из портов (используется peek для безопасности).
     *
     * @see peek(), route_pkt(), RequestsList
     */
    void collect_requests(AllRequests & requests);

    /**
     * @brief Фаза 2: Арбитраж конфликтов на выходных портах.
     * @param[in] requests Карта запросов от collect_requests().
     * @param[out] senders_list Вектор победителей: senders_list[out_idx] = индекс входа-победителя.
     *
     * @details
     * Для каждого выходного порта:
     * - Если есть запросы, вызывается arbiter.arbitrate() для выбора победителя
     * - Индекс победившего входного порта сохраняется в senders_list
     * - Если запросов нет, senders_list[out_idx] остается -1
     *
     * @pre requests.size() == out_port_count
     * @pre senders_list.size() == out_port_count
     * @post senders_list содержит индексы победителей для каждого выхода.
     *
     * @see RRArbiter::arbitrate(), RequestsList
     */
    void arbitrate_all(const AllRequests & requests, std::vector<int> & senders_list);

    /**
     * @brief Фаза 3: Фактическая отправка пакетов победителям.
     * @param[in] senders_list Список победителей от arbitrate_all().
     *
     * @details
     * Для каждого выходного порта:
     * - Если выход готов (isReady) и есть победитель (senders_list[out_idx] != -1):
     *   1. Извлекает пакет из входного порта победителя (tryRecv)
     *   2. Отправляет пакет в выходной порт (trySend)
     * - Если выход занят: пакет остается во входном буфере (backpressure)
     *
     * @pre senders_list.size() == out_port_count
     * @post Пакеты переданы на свободные выходы; занятые выходы сохраняют пакеты на входах.
     *
     * @note Реализует механизм backpressure: потеря пакетов невозможна.
     * @see Port::tryRecv(), Port::trySend(), Port::isReady()
     */
    void send_all(const std::vector<int> & senders_list);

    /**
     * @brief Проверить наличие входного порта по индексу.
     * @param[in] idx индекс входного порта
     * @return true если порт существует
     */
    [[nodiscard]] inline bool has_in_port(int idx) const;

    /**
     * @brief Проверить наличие выходного порта по индексу.
     * @param[in] idx индекс выходного порта
     * @return true если порт существует
     */
    [[nodiscard]] inline bool has_out_port(int idx) const;

    /**
     * @brief Выставить бит наличия входного порта.
     * @param[in] idx индекс входного порта
     * @param[in] exists существует/пустой
     * @see in_ports_mask
     */
    void set_in_port_exists(int idx, bool exists = true);

    /**
     * @brief Выставить бит наличия выходного порта.
     * @param[in] idx индекс выходного порта
     * @param[in] exists существует/пустой
     * @see out_ports_mask
     */
    void set_out_port_exists(int idx, bool exists = true);

    // === Геттеры ===

    [[nodiscard]] int get_id()           const { return id; }
    [[nodiscard]] int get_input_count()  const { return in_port_count; }
    [[nodiscard]] int get_output_count() const { return out_port_count; }
};

// Include template implementations
#include "router.tpp"
