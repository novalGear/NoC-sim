/**
 * @file test_port.cpp
 * @brief Unit-тесты для класса Port с использованием Google Test
 * @author Novoselov Alexander
 * @date 18/03/2026
 */

#include <gtest/gtest.h>
#include <limits.h>
#include "port.hpp"

// Тест: начальное состояние порта
TEST(PortTest, InitialState) {
    Port port;

    EXPECT_FALSE(port.has_data());
    EXPECT_TRUE(port.is_ready());
    EXPECT_FALSE(port.peek().has_value());
    EXPECT_FALSE(port.try_recv().has_value());
}

// Тест: try_send на пустой порт
TEST(PortTest, TrySendOnEmptyPort) {
    Port port;
    Packet test_pkt(1, 2, 3);  // id=1, src=2, dst=3

    bool sent = port.try_send(test_pkt);

    EXPECT_TRUE(sent);
    EXPECT_TRUE(port.has_data());
    EXPECT_FALSE(port.is_ready());
}

// Тест: try_send на занятый порт (backpressure)
TEST(PortTest, TrySendOnOccupiedPort) {
    Port port;
    Packet pkt1(1, 2, 3);
    Packet pkt2(4, 5, 6);

    // Первая отправка успешна
    ASSERT_TRUE(port.try_send(pkt1));

    // Вторая должна провалиться
    bool sent = port.try_send(pkt2);
    EXPECT_FALSE(sent);

    // Оригинальный пакет должен остаться
    auto peeked = port.peek();
    ASSERT_TRUE(peeked.has_value());
    EXPECT_EQ(peeked->id, 1);
}

// Тест: try_recv на пустом порту
TEST(PortTest, TryRecvOnEmptyPort) {
    Port port;

    auto result = port.try_recv();
    EXPECT_FALSE(result.has_value());
}

// Тест: try_recv после отправки
TEST(PortTest, TryRecvAfterSend) {
    Port port;
    Packet test_pkt(1, 2, 3);

    port.try_send(test_pkt);

    auto result = port.try_recv();
    ASSERT_TRUE(result.has_value());

    // Проверяем целостность пакета
    Packet received = result.value();
    EXPECT_EQ(received.id, 1);
    EXPECT_EQ(received.src, 2);
    EXPECT_EQ(received.dst, 3);
}

// Тест: try_recv очищает порт
TEST(PortTest, TryRecvClearsPort) {
    Port port;
    Packet test_pkt(1, 2, 3);

    port.try_send(test_pkt);
    port.try_recv();

    EXPECT_FALSE(port.has_data());
    EXPECT_TRUE(port.is_ready());
}

// Тест: двойной try_recv
TEST(PortTest, DoubleTryRecv) {
    Port port;
    Packet test_pkt(1, 2, 3);

    port.try_send(test_pkt);

    auto first = port.try_recv();
    EXPECT_TRUE(first.has_value());

    auto second = port.try_recv();
    EXPECT_FALSE(second.has_value());
}

// Тест: peek на пустом порту
TEST(PortTest, PeekOnEmptyPort) {
    Port port;

    auto result = port.peek();
    EXPECT_FALSE(result.has_value());
}

// Тест: peek возвращает пакет без удаления
TEST(PortTest, PeekReturnsWithoutRemoving) {
    Port port;
    Packet test_pkt(1, 2, 3);

    port.try_send(test_pkt);

    auto peek1 = port.peek();
    ASSERT_TRUE(peek1.has_value());
    EXPECT_EQ(peek1->id, 1);

    // Порт все еще содержит данные
    EXPECT_TRUE(port.has_data());

    auto peek2 = port.peek();
    ASSERT_TRUE(peek2.has_value());
    EXPECT_EQ(peek2->id, 1);

    // Можно извлечь после peek
    auto recvd = port.try_recv();
    EXPECT_TRUE(recvd.has_value());
}

// Тест: backpressure сценарий
TEST(PortTest, BackpressureScenario) {
    Port port;
    Packet pkt1(1, 2, 3);
    Packet pkt2(4, 5, 6);

    // Нельзя отправить когда полный
    EXPECT_TRUE(port.try_send(pkt1));
    EXPECT_FALSE(port.try_send(pkt2));

    // Можно отправить после извлечения
    port.try_recv();
    EXPECT_TRUE(port.try_send(pkt2));

    auto result = port.try_recv();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id, 4);
}

// Тест: независимость пакетов (копирование)
TEST(PortTest, PacketIndependence) {
    Port port;
    Packet original(1, 2, 3);

    // Сохраненный пакет - копия
    port.try_send(original);

    // Модифицируем оригинал
    original.id = 99;

    auto stored = port.peek();
    ASSERT_TRUE(stored.has_value());
    EXPECT_EQ(stored->id, 1);  // Не изменился
}

// Тест: краевые случаи
TEST(PortTest, EdgeCases) {
    Port port;

    // try_recv на пустом, затем try_send
    port.try_recv();  // Должно быть безопасно
    EXPECT_TRUE(port.try_send(Packet(1, 2, 3)));

    // Множественные peek не меняют состояние
    for (int i = 0; i < 5; ++i) {
        auto peeked = port.peek();
        ASSERT_TRUE(peeked.has_value());
        EXPECT_EQ(peeked->id, 1);
    }

    EXPECT_TRUE(port.has_data());
    EXPECT_FALSE(port.is_ready());
}

// Тест: пакет с максимальными значениями
TEST(PortTest, PacketWithMaxValues) {
    Port port;
    Packet pkt(INT_MAX, INT_MAX, INT_MAX);

    EXPECT_TRUE(port.try_send(pkt));

    auto received = port.try_recv();
    ASSERT_TRUE(received.has_value());
    EXPECT_EQ(received->id, INT_MAX);
    EXPECT_EQ(received->src, INT_MAX);
    EXPECT_EQ(received->dst, INT_MAX);
}

// Тест: множество операций подряд
TEST(PortTest, MultipleOperations) {
    Port port;

    for (int i = 0; i < 10; ++i) {
        Packet pkt(i, i+1, i+2);

        // Отправляем
        EXPECT_TRUE(port.try_send(pkt));
        EXPECT_TRUE(port.has_data());
        EXPECT_FALSE(port.is_ready());

        // Проверяем через peek
        auto peeked = port.peek();
        ASSERT_TRUE(peeked.has_value());
        EXPECT_EQ(peeked->id, i);

        // Извлекаем
        auto received = port.try_recv();
        ASSERT_TRUE(received.has_value());
        EXPECT_EQ(received->id, i);

        // Порт снова пуст
        EXPECT_FALSE(port.has_data());
        EXPECT_TRUE(port.is_ready());
    }
}
