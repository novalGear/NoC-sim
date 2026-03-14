/**
 * @file rr_arbiter.hpp
 * @brief Реализация арбитра Round-Robin для разрешения конфликтов в сетях-on-chip.
 * @author Novoselov Alexander
 * @date   14/03/2026
 */

#pragma once

#include <vector>
#include <cassert>
#include "packet.hpp" // Предполагается, что структура Request определена здесь

/**
 * @class RRArbiter
 * @brief Арбитр с алгоритмом Round-Robin (циклическое обслуживание).
 *
 * Этот класс реализует справедливый алгоритм арбитража, который предотвращает голодание (starvation)
 * любых входных портов. Алгоритм выбирает запросчика, ближайшего к текущей "голове" (head)
 * в циклическом порядке, и сдвигает голову на следующую позицию после победителя.
 *
 * @details
 * Алгоритм работы:
 * 1. Для каждого запроса вычисляется циклическое расстояние от текущей головы:
 *    `dist = (port_idx - head + N) % N`.
 * 2. Выбирается запрос с минимальным расстоянием.
 * 3. Голова обновляется: `head = (winner_port_idx + 1) % N`.
 *
 * Пример использования:
 * @code
 * RRArbiter arbiter(4); // 4 входных порта
 * std::vector<Request> reqs = {{0}, {2}}; // Запросы от портов 0 и 2
 * int winner_idx = arbiter.arbitrate(reqs, 0); // Возвращает индекс в векторе reqs
 * @endcode
 */
class RRArbiter {
private:
    int head_port_idx;      ///< Индекс "головы" цикла Round-Robin. Следующий приоритетный порт.
    int src_ports_count;    ///< Общее количество обслуживаемых входных портов.

    /**
     * @brief Вычисляет циклическое расстояние от головы до заданного порта.
     * @param[in] port_idx Индекс физического порта.
     * @return Расстояние (количество шагов) от головы до порта по часовой стрелке.
     */
    [[nodiscard]] int calc_port_distance(int port_idx) const {
        assert(port_idx >= 0 && port_idx < src_ports_count);
        return (port_idx + src_ports_count - head_port_idx) % src_ports_count;
    }

    /**
     * @brief Обновляет указатель головы после выбора победителя.
     * @param[in] winner_port_idx Индекс физического порта-победителя.
     */
    void upd_head(int winner_port_idx) {
        assert(winner_port_idx >= 0 && winner_port_idx < src_ports_count);
        head_port_idx = (winner_port_idx + 1) % src_ports_count;
    }

public:
    /**
     * @brief Конструктор арбитра.
     * @param[in] ports_count Количество входных портов. Должно быть больше нуля.
     *
     * @pre `ports_count > 0`
     * @post Инициализирует `src_ports_count` и сбрасывает `head_port_idx` в 0.
     */
    explicit RRArbiter(int ports_count)
        : head_port_idx(0), src_ports_count(ports_count)
    {
        assert(ports_count > 0);
    }

    /**
     * @brief Выполняет арбитраж среди списка запросов.
     * @param[in] requests Вектор активных запросов от входных портов.
     * @param[in] outputPortId Идентификатор выходного порта (для отладки/логирования, не используется в логике).
     * @return Индекс победившего запроса в векторе `requests`, или `-1`, если запросов нет.
     *
     * @details
     * Проходит по всем запросам, вычисляет расстояние до головы и выбирает ближайший.
     * После выбора обновляет внутреннее состояние (голову) для обеспечения справедливости Round-Robin.
     *
     * @note Возвращаемое значение — это индекс в векторе `requests`, а не номер физического порта!
     *       Чтобы получить номер порта: `requests[result].src_port_idx`.
     *
     * @retval -1 Если вектор `requests` пуст.
     * @retval >=0 Индекс элемента в векторе `requests`, который выиграл арбитраж.
     *
     * @see calc_port_distance(), upd_head()
     */
    [[nodiscard]] int arbitrate(const std::vector<Request>& requests, int outputPortId) {
        if (requests.empty()) { return -1; }

        int winner_req_idx = 0;
        int min_dist = src_ports_count + 1;

        for (int i = 0; i < static_cast<int>(requests.size()); ++i) {
            int dist = calc_port_distance(requests[i].src_port_idx);

            if (dist < min_dist) {
                min_dist = dist;
                winner_req_idx = i;
            }
        }

        upd_head(requests[winner_req_idx].src_port_idx);
        return winner_req_idx;
    }
};
