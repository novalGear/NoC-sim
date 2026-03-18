/**
 * @file port.hpp
 * @brief Порт интерконнекта — базовый элемент хранения пакета с сигналом валидности.
 * @author Novoselov Alexander
 * @date 14/03/2026
 */

#pragma once

#include "packet.hpp"
#include <optional>

/**
 * @class Port
 * @brief Представляет аппаратный порт с буфером на один пакет.
 *
 * Порт моделирует цикл-точное поведение аппаратного буфера:
 * - Хранит не более одного пакета (флит-уровень)
 * - Реализует сигналы valid/ready для backpressure
 */
class Port {
private:
    /** @brief Внутренний буфер пакета. Пустой = порт свободен. */
    std::optional<Packet> buffer;

    /** @brief Очищает буфер порта (устанавливает valid = false). */
    void clr_buffer() {
        buffer.reset();
    }

public:
    /**
     * @brief Проверяет наличие данных в порту (сигнал valid).
     * @return true если буфер содержит пакет, false если пуст.
     */
    [[nodiscard]] bool hasData() const {
        return buffer.has_value();
    }

    /**
     * @brief Проверяет готовность порта к приему нового пакета (сигнал ready).
     * @return true если буфер пуст и порт готов принять данные.
     */
    [[nodiscard]] bool isReady() const {
        return !buffer.has_value();
    }

    /**
     * @brief Попытка отправить пакет в порт.
     * @param[in] pkt Ссылка на пакет для отправки (копируется в буфер).
     * @return true если отправка успешна, false если порт занят (backpressure).
     */
    [[nodiscard]] bool trySend(const Packet& pkt) {
        if (isReady()) {
            buffer = pkt;
            return true;
        }
        return false;
    }

    /**
     * @brief Забирает пакет из порта (потребляет данные).
     * @return std::optional<Packet> с пакетом если данные были, или nullopt если порт пуст.
     */
    [[nodiscard]] std::optional<Packet> tryRecv() {
        if (hasData()) {
            Packet pkt = buffer.value();
            clr_buffer();
            return pkt;
        }
        return std::nullopt;
    }

    /**
     * @brief Просмотр пакета без извлечения (недеструктивное чтение).
     * @return std::optional<Packet> с пакетом если данные есть, или nullopt.
     * @see tryRecv(), hasData()
     */
    [[nodiscard]] std::optional<Packet> peek() const {
        return buffer;
    }
};
