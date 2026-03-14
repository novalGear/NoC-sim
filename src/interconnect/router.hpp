/**
 * @file router.hpp
 * @brief Класс маршрутизатора (Router) для симуляции интерконнекта с арбитражем Round-Robin.
 * @author Novoselov Alexandr
 * @date   14/03/2026
 */

#pragma once

#include "packet.hpp"
#include "port.hpp"
#include "rr_arbiter.hpp"

#include <vector>
#include <optional>
#include <cassert>

/**
 * @class Router
 * @brief Маршрутизатор с арбитражем Round-Robin для управления потоком пакетов.
 *
 * @details
 * Router моделирует цикл-точное поведение аппаратного маршрутизатора в сети-on-chip:
 * - Имеет фиксированное количество входных и выходных портов
 * - На каждом такте (on_clock) выполняет трехфазный алгоритм:
 *   1. **Collect**: Сбор заявок от входных портов (без извлечения пакетов)
 *   2. **Arbitrate**: Разрешение конфликтов через RRArbiter (Round-Robin)
 *   3. **Send**: Фактическая передача выигравших пакетов на выходные порты
 *
 * Механизм backpressure:
 * - Пакет извлекается из входного порта только если выходной порт готов (isReady)
 * - Если выход занят, пакет остается во входном буфере до следующего такта
 *
 * @note Класс не потокобезопасен. Все вызовы должны происходить из одного потока симуляции.
 * @note Метод route_pkt() является заглушкой и должен быть переопределен для конкретной топологии.
 *
 * @see Port, RRArbiter, Packet, Interconnect
 */
class Router {
private:
    int id;  ///< Уникальный идентификатор маршрутизатора в сети

    std::vector<Port> input_ports;   ///< Входные порты маршрутизатора
    std::vector<Port> output_ports;  ///< Выходные порты маршрутизатора

    int in_port_count;   ///< Количество входных портов (кэшировано для производительности)
    int out_port_count;  ///< Количество выходных портов (кэшировано для производительности)

    RRArbiter arbiter;   ///< Арбитр для разрешения конфликтов на выходных портах

    /**
     * @brief Простая заглушка маршрутизации: всегда возвращает порт 0.
     * @param[in] pkt Пакет для маршрутизации.
     * @return Индекс выходного порта в диапазоне [0, out_port_count).
     *
     * @note В реальной реализации должен учитывать топологию сети и адрес назначения пакета.
     * @see MeshRouter::route_pkt(), ButterflyRouter::route_pkt()
     */
    [[nodiscard]] int route_pkt(const Packet& pkt) const;

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
     * @pre requests должен быть пустым или будет перезапписан.
     * @post requests[out_idx] содержит индексы входов, желающих отправить на out_idx.
     * @note Пакеты НЕ извлекаются из портов (используется peek для безопасности).
     *
     * @see peek(), route_pkt(), RequestsList
     */
    void collect_requests(AllRequests& requests);

    /**
     * @brief Фаза 2: Арбитраж конфликтов на выходных портах.
     * @param[in] requests Карта запросов от collect_requests().
     * @param[out] senders_list Вектор победителей: senders_list[out_idx] = индекс входа-победителя.
     *
     * @details
     * Для каждого выходного порта:
     * - Если есть запросы, вызывается arbiter.arbitrate() для выбора победителя
     * - Индекс победившего входного порта сохраняется в senders_list
     * - Если запросов нет, senders_list[out_idx] остается неопределенным (не используется)
     *
     * @pre requests.size() == out_port_count
     * @post senders_list содержит индексы победителей для каждого выхода.
     *
     * @see RRArbiter::arbitrate(), RequestsList
     */
    void arbitrate_all(const AllRequests& requests, std::vector<int>& senders_list);

    /**
     * @brief Фаза 3: Фактическая отправка пакетов победителям.
     * @param[in] senders_list Список победителей от arbitrate_all().
     *
     * @details
     * Для каждого выходного порта:
     * - Если выход готов (isReady) и есть победитель:
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
    void send_all(const std::vector<int>& senders_list);

    // Геттеры для отладки и тестирования
    [[nodiscard]] int getId() const { return id; }
    [[nodiscard]] int getInputCount() const { return in_port_count; }
    [[nodiscard]] int getOutputCount() const { return out_port_count; }

    /**
     * @brief Получить доступ к входному порту (для тестов/инжекции пакетов).
     * @param[in] idx Индекс порта [0, in_port_count).
     * @return Ссылка на порт.
     * @pre 0 <= idx < in_port_count
     */
    Port& getInputPort(int idx) {
        assert(idx >= 0 && idx < in_port_count);
        return input_ports[idx];
    }

    /**
     * @brief Получить доступ к выходному порту (для тестов/сбора статистики).
     * @param[in] idx Индекс порта [0, out_port_count).
     * @return Ссылка на порт.
     * @pre 0 <= idx < out_port_count
     */
    Port& getOutputPort(int idx) {
        assert(idx >= 0 && idx < out_port_count);
        return output_ports[idx];
    }
};
