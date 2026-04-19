/**
 * @file router.hpp
 * @brief Абстрактный базовый класс маршрутизатора для симуляции интерконнекта.
 * @author Novoselov Alexandr
 * @date   16/03/2026
 */

#pragma once

#include "packet.hpp"
#include "port.hpp"
#include "rr_arbiter.hpp"
#include "debug.hpp"

#include <vector>
#include <optional>
#include <cassert>
#include <memory>

constexpr size_t MAX_PORTS = 64;
using PortMask = std::bitset<MAX_PORTS>;

/**
 * @class Router
 * @brief Абстрактный базовый класс маршрутизатора с арбитражем Round-Robin.
 *
 * @details
 * Router определяет общий интерфейс и реализует цикл-точное поведение
 * аппаратного маршрутизатора в сети-on-chip:
 * - Имеет фиксированное количество входных и выходных портов
 * - На каждом такте (on_clock) выполняет трехфазный алгоритм:
 *   1. **Collect**: Сбор заявок от входных портов (без извлечения пакетов)
 *   2. **Arbitrate**: Разрешение конфликтов через RRArbiter (Round-Robin)
 *   3. **Send**: Фактическая передача выигравших пакетов на выходные порты
 *
 * Наследники (например, MeshRouter, ButterflyRouter) обязаны реализовать:
 * - Логику маршрутизации пакетов (метод route_pkt())
 * - Специфичную для топологии инициализацию портов
 *
 * Механизм backpressure:
 * - Пакет извлекается из входного порта только если выходной порт готов (isReady)
 * - Если выход занят, пакет остается во входном буфере до следующего такта
 *
 * @note Класс не поддерживает копирование (delete), но поддерживает перемещение.
 * @note Все вызовы должны происходить из одного потока симуляции.
 *
 * @see Port, RRArbiter, Packet, Interconnect, MeshRouter, ButterflyRouter
 */
class Router {
private:
    int id;  ///< Уникальный идентификатор маршрутизатора в сети

protected:
    std::vector<Port*> input_ports;   ///< Входные порты маршрутизатора
    std::vector<Port*> output_ports;  ///< Выходные порты маршрутизатора

    int in_port_count;   ///< Количество входных портов (кэшировано для производительности)
    int out_port_count;  ///< Количество выходных портов (кэшировано для производительности)

    PortMask in_ports_mask;    ///< Маска существования входных портов
    PortMask out_ports_mask;   ///< Маска существования выходных портов

    RRArbiter arbiter;   ///< Арбитр для разрешения конфликтов на выходных портах

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
    Router(int input_count, int output_count, int router_id = 0)
        : id(router_id)
        , input_ports(input_count)
        , output_ports(output_count)
        , in_port_count(input_count)
        , out_port_count(output_count)
        , arbiter(input_count)  // Арбитр работает с input_count источниками
    {
        assert(input_count > 0 && "Input port count must be positive");
        assert(output_count > 0 && "Output port count must be positive");
    }

    /**
     * @brief Виртуальный деструктор.
     * @details Обеспечивает корректное удаление объектов наследников.
     */
    virtual ~Router() = default;

    // Запрет копирования
    Router(const Router&) = delete;
    Router& operator=(const Router&) = delete;

    // Разрешение перемещения
    Router(Router&&) noexcept = default;
    Router& operator=(Router&&) noexcept = default;

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
    void on_clock() {
        AllRequests requests(out_port_count);  // Вектор списков запросов для каждого выхода
        std::vector<int> senders_list(out_port_count, -1);  // -1 означает "нет победителя"

        collect_requests(requests);
        arbitrate_all(requests, senders_list);
        send_all(senders_list);
    }

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
    void collect_requests(AllRequests& requests) {
        // Очищаем списки запросов
        for (auto& req_list : requests) {
            req_list.clear();
        }

        // Сканируем все входные порты
        for (int in_idx = 0; in_idx < in_port_count; ++in_idx) {
            if (input_ports[in_idx] == nullptr) {
                continue;  // Пропускаем незарегистрированные порты
            }

            auto pkt_opt = input_ports[in_idx]->peek();
            if (pkt_opt.has_value()) {
                // Определяем целевой выход для пакета
                int out_idx = route_pkt(pkt_opt.value());
                DEBUG_PRINT("Router " << id << ": packet from in_port[" << in_idx
                       << "] -> out_port[" << out_idx << "] (dst=" << pkt_opt->dst << ")");

                // Добавляем запрос от этого входного порта
                assert(has_out_port(out_idx) &&
                    "Route function returned invalid port index");
                requests[out_idx].emplace_back(in_idx);
            }
        }
    }

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
    void arbitrate_all(const AllRequests& requests, std::vector<int>& senders_list) {
        assert(requests.size() == static_cast<size_t>(out_port_count));
        assert(senders_list.size() == static_cast<size_t>(out_port_count));

        for (int out_idx = 0; out_idx < out_port_count; ++out_idx) {
            if (!requests[out_idx].empty()) {
                // Арбитр возвращает индекс в векторе requests[out_idx]
                int winner_req_idx = arbiter.arbitrate(requests[out_idx], out_idx);
                assert(winner_req_idx >= 0 && winner_req_idx < static_cast<int>(requests[out_idx].size()));

                // Сохраняем физический индекс входного порта-победителя
                senders_list[out_idx] = requests[out_idx][winner_req_idx].src;
            }
            // else: senders_list[out_idx] уже -1 (инициализировано в on_clock)
        }
    }

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
    void send_all(const std::vector<int>& senders_list) {
        assert(senders_list.size() == static_cast<size_t>(out_port_count));

        for (int out_idx = 0; out_idx < out_port_count; ++out_idx) {
            int in_idx = senders_list[out_idx];

            if (not has_out_port(out_idx)) {
                if (in_idx != -1) {
                    DEBUG_PRINT("ERROR: Trying to send pkt from in_port[" << in_idx << "] to an empty out_port[" << out_idx << "]!");
                }
                continue;
            }

            // Если есть победитель и выходной порт готов принять пакет
            if (in_idx != -1 && output_ports[out_idx]->is_ready()) {
                assert(input_ports[in_idx] != nullptr);
                // Пытаемся извлечь пакет из входного порта победителя
                auto pkt_opt = input_ports[in_idx]->try_recv();
                assert(pkt_opt.has_value() && "Winner should have a packet");

                // Отправляем пакет в выходной порт
                bool sent = output_ports[out_idx]->try_send(pkt_opt.value());
                DEBUG_PRINT("Router " << id << ": moving packet from in_port[" << in_idx
                       << "] to out_port[" << out_idx << "] (id=" << pkt_opt->id << ")");
                assert(sent && "Output port should be ready (checked with isReady)");
            }
            // Если выход занят, пакет остается во входном порту (backpressure)
        }
    }
    /**
     * @brief Проверить наличие входного порта по индексу.
     * @param[in] idx индекс входного порта
     * @return true если порт существует
     */
    [[nodiscard]] inline bool has_in_port(int idx) const {
        assert(0 <= idx && idx < in_port_count);
        return in_ports_mask.test(idx);
    }

    /**
     * @brief Проверить наличие выходного порта по индексу.
     * @param[in] idx индекс выходного порта
     * @return true если порт существует
     */
    [[nodiscard]] inline bool has_out_port(int idx) const {
        assert(0 <= idx && idx < out_port_count);
        return out_ports_mask.test(idx);
    }

    /**
     * @brief Выставить бит наличия входного порта .
     * @param[in] idx индекс входного порта
     * @param[in] exists существует/пустой
     * @see in_ports_mask
     */
    void set_in_port_exists(int idx, bool exists = true) {
        assert(idx >= 0 && idx < in_port_count);
        if (idx >= 0 && idx < in_port_count) {
            in_ports_mask.set(idx, exists);
        }
    }

    /**
     * @brief Выставить бит наличия выходного порта .
     * @param[in] idx индекс выходного порта
     * @param[in] exists существует/пустой
     * @see out_ports_mask
     */
    void set_out_port_exists(int idx, bool exists = true) {
        assert(idx >= 0 && idx < out_port_count);
        if (idx >= 0 && idx < out_port_count) {
            out_ports_mask.set(idx, exists);
        }
    }

    // === Геттеры ===

    [[nodiscard]] int get_id()           const { return id; }
    [[nodiscard]] int get_input_count()  const { return in_port_count; }
    [[nodiscard]] int get_output_count() const { return out_port_count; }
};
