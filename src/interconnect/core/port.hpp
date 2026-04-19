/**
 * @file port.hpp
 * @brief Порт интерконнекта с задержкой на один такт.
 * @author Novoselov Alexander
 * @date 19/04/2026
 */

#pragma once

#include "packet.hpp"
#include <optional>

/**
 * @class Port
 * @brief Представляет аппаратный порт с задержкой на один такт.
 *
 * Порт моделирует цикл-точное поведение аппаратного буфера:
 * - Пакет, записанный в такте T, становится доступным для чтения в такте T+1
 * - Реализует сигналы valid/ready для backpressure
 */
class Port {
private:
    /** @brief Пакет, который будет прочитан в следующем такте */
    std::optional<Packet> recv_buffer;

    /** @brief Пакет, записанный в текущем такте */
    std::optional<Packet> send_buffer;

public:
    /**
     * @brief Такт порта — перекладывает send_buffer в recv_buffer.
     * Должен вызываться после того, как все роутеры завершили send_all.
     */
    void on_clock() {
        // Перемещаем send_buffer в recv_buffer ТОЛЬКО если recv_buffer пуст
        if (!has_data() && send_buffer.has_value()) {
            recv_buffer = std::move(send_buffer);
            send_buffer.reset();
        }
    }

    /**
     * @brief Проверяет наличие данных для чтения.
     * @return true если recv_buffer содержит пакет.
     */
    [[nodiscard]] bool has_data() const {
        return recv_buffer.has_value();
    }

    /**
     * @brief Проверяет готовность к приему нового пакета.
     * @return true если send_buffer пуст.
     */
    [[nodiscard]] bool is_ready() const {
        return !send_buffer.has_value();
    }

    /**
     * @brief Попытка отправить пакет в порт.
     * @param[in] pkt Пакет для отправки.
     * @return true если отправка успешна (send_buffer был пуст).
     * @note Пакет станет доступен для чтения на следующем такте.
     */
    [[nodiscard]] bool try_send(const Packet& pkt) {
        if (is_ready()) {
            send_buffer = pkt;
            return true;
        }
        return false;
    }

    /**
     * @brief Забирает пакет из порта.
     * @return Пакет, если recv_buffer не пуст.
     * @note Читает пакет, записанный в предыдущем такте.
     */
    [[nodiscard]] std::optional<Packet> try_recv() {
        if (has_data()) {
            Packet pkt = recv_buffer.value();
            recv_buffer.reset();
            return pkt;
        }
        return std::nullopt;
    }

    /**
     * @brief Просмотр пакета без извлечения.
     * @return Пакет из recv_buffer, если есть.
     */
    [[nodiscard]] std::optional<Packet> peek() const {
        return recv_buffer;
    }
};
