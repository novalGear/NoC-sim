/**
 * @file router.cpp
 * @brief Реализация методов класса Router.
 */

#include "router.hpp"

Router::Router(int input_count, int output_count, int router_id)
    : id(router_id),
      input_ports(input_count),
      output_ports(output_count),
      in_port_count(input_count),
      out_port_count(output_count),
      arbiter(input_count)  // Арбитр знает количество входов для Round-Robin
{
    assert(input_count > 0 && output_count > 0);
}

int Router::route_pkt(const Packet& pkt) const {
    // ЗАГЛУШКА: Простая маршрутизация "всё на порт 0"
    // В реальной реализации:
    // - Для Mesh: вычисление направления по координатам (XY-routing)
    // - Для Butterfly: маршрутизация по битам адреса назначения
    // - Для адаптивной: учет загруженности каналов

    (void)pkt; // Подавление warning о неиспользуемом параметре в заглушке
    return 0;  // Всегда отправляем на выходной порт 0
}

void Router::on_clock() {
    AllRequests requests;
    std::vector<int> senders_list;

    collect_requests(requests);
    arbitrate_all(requests, senders_list);
    send_all(senders_list);
}

void Router::collect_requests(AllRequests& requests) {
    requests.clear();
    requests.resize(out_port_count);

    for (int in_idx = 0; in_idx < in_port_count; ++in_idx) {
        Port& in_port = input_ports[in_idx];

        if (in_port.hasData()) {
            auto pkt_opt = in_port.peek();
            assert(pkt_opt.has_value());  // hasData() гарантирует наличие

            int dst_port = route_pkt(pkt_opt.value());
            // Валидация: целевой порт должен быть в допустимом диапазоне
            assert(dst_port >= 0 && dst_port < out_port_count);

            requests[dst_port].push_back(in_idx);
        }
    }
}

void Router::arbitrate_all(const AllRequests& requests, std::vector<int>& senders_list) {
    // Инициализируем список победителей
    senders_list.resize(out_port_count, -1);  // -1 означает "нет победителя"

    for (int out_idx = 0; out_idx < out_port_count; ++out_idx) {
        const RequestsList& req_list = requests[out_idx];

        // Если на этот выход нет запросов — пропускаем
        if (req_list.empty()) { continue; }

        // Запускаем арбитраж: получаем индекс победителя в векторе запросов
        int winner_req_idx = arbiter.arbitrate(req_list, out_idx);

        // Если арбитр вернул валидный индекс, сохраняем физический номер входа
        if (winner_req_idx >= 0 && winner_req_idx < static_cast<int>(req_list.size())) {
            senders_list[out_idx] = req_list[winner_req_idx];
        }
    }
}

void Router::send_all(const std::vector<int>& senders_list) {
    for (int out_idx = 0; out_idx < out_port_count; ++out_idx) {
        Port& out_port = output_ports[out_idx];

        // Если out порт не готов к отправке - пропускаем
        if (!out_port.isReady()) { continue; }
        int in_idx = senders_list[out_idx];

        // Если нет победителя для этого выхода — пропускаем
        if (in_idx < 0 || in_idx >= in_port_count) { continue; }

        Port& in_port = input_ports[in_idx];

        if (!in_port.hasData()) {
            continue;  // Защитная проверка на случай гонки состояний
        }

        // Извлекаем пакет из входа и отправляем на выход
        auto pkt_opt = in_port.tryRecv();
        if (pkt_opt.has_value()) {
            bool sent = out_port.trySend(pkt_opt.value());
            // trySend должен вернуть true, т.к. мы проверили isReady() выше
            (void)sent; // Подавление warning, в отладке можно добавить assert(sent)
        }
    }
}
