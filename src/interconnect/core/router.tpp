/**
@file router.tpp
@brief Implementation of the templated Router class.
*/
#pragma once

template <typename ArbiterT>
Router<ArbiterT>::Router(int input_count, int output_count, int router_id)
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

template <typename ArbiterT>
void Router<ArbiterT>::on_clock() {
    AllRequests requests(out_port_count);  // Вектор списков запросов для каждого выхода
    std::vector<int> senders_list(out_port_count, -1);  // -1 означает "нет победителя"

    collect_requests(requests);
    arbitrate_all(requests, senders_list);
    send_all(senders_list);
}

template <typename ArbiterT>
void Router<ArbiterT>::collect_requests(AllRequests & requests) {
    // Очищаем списки запросов
    for (auto & req_list : requests) {
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

template <typename ArbiterT>
void Router<ArbiterT>::arbitrate_all(const AllRequests & requests, std::vector<int> & senders_list) {
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

template <typename ArbiterT>
void Router<ArbiterT>::send_all(const std::vector<int> & senders_list) {
    assert(senders_list.size() == static_cast<size_t>(out_port_count));

    for (int out_idx = 0; out_idx < out_port_count; ++out_idx) {
        int in_idx = senders_list[out_idx];

        if (!has_out_port(out_idx)) {
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
            pkt_opt->hops++;

            // Отправляем пакет в выходной порт
            bool sent = output_ports[out_idx]->try_send(pkt_opt.value());
            DEBUG_PRINT("Router " << id << ": moving packet from in_port[" << in_idx
                    << "] to out_port[" << out_idx << "] (id=" << pkt_opt->id << ")");
            assert(sent && "Output port should be ready (checked with isReady)");
        }
        // Если выход занят, пакет остается во входном порту (backpressure)
    }
}

template <typename ArbiterT>
[[nodiscard]] inline bool Router<ArbiterT>::has_in_port(int idx) const {
    assert(0 <= idx && idx < in_port_count);
    return in_ports_mask.test(idx);
}

template <typename ArbiterT>
[[nodiscard]] inline bool Router<ArbiterT>::has_out_port(int idx) const {
    assert(0 <= idx && idx < out_port_count);
    return out_ports_mask.test(idx);
}

template <typename ArbiterT>
void Router<ArbiterT>::set_in_port_exists(int idx, bool exists) {
    assert(idx >= 0 && idx < in_port_count);
    if (idx >= 0 && idx < in_port_count) {
        in_ports_mask.set(idx, exists);
    }
}

template <typename ArbiterT>
void Router<ArbiterT>::set_out_port_exists(int idx, bool exists) {
    assert(idx >= 0 && idx < out_port_count);
    if (idx >= 0 && idx < out_port_count) {
        out_ports_mask.set(idx, exists);
    }
}
