/**
 * @file port_test.cpp
 * @brief Google Test для класса Port.
 * @author Novoselov Alexander
 * @date 14/03/2026
 */

#include <gtest/gtest.h>
#include "../src/port.hpp"
#include "../src/packet.hpp"

/**
 * @class PortTest
 * @brief Фикстура для тестов класса Port.
 */
class PortTest : public ::testing::Test {
protected:
    Port port;
    Packet test_pkt{42, 100, 200};

    void SetUp() override {
        // Перед каждым тестом порт гарантированно пуст
        ASSERT_FALSE(port.hasData());
        ASSERT_TRUE(port.isReady());
    }
};

// === Тесты состояния ===

TEST_F(PortTest, InitialState) {
    EXPECT_FALSE(port.hasData());
    EXPECT_TRUE(port.isReady());
    EXPECT_FALSE(port.peek().has_value());
}

// === Тесты отправки (trySend) ===

TEST_F(PortTest, SendToEmptyPort) {
    EXPECT_TRUE(port.trySend(test_pkt));
    EXPECT_TRUE(port.hasData());
    EXPECT_FALSE(port.isReady());
}

TEST_F(PortTest, SendToFullPort_Backpressure) {
    // Заполняем порт
    ASSERT_TRUE(port.trySend(test_pkt));

    // Второй пакет должен быть отклонен
    Packet another_pkt{99, 1, 2};
    EXPECT_FALSE(port.trySend(another_pkt));

    // Исходный пакет не должен измениться
    auto peeked = port.peek();
    ASSERT_TRUE(peeked.has_value());
    EXPECT_EQ(peeked->id, 42);
}

// === Тесты чтения (peek / tryRecv) ===

TEST_F(PortTest, PeekDoesNotConsume) {
    port.trySend(test_pkt);

    // Первый peek
    auto p1 = port.peek();
    ASSERT_TRUE(p1.has_value());
    EXPECT_EQ(p1->id, 42);

    // Порт все еще полон
    EXPECT_TRUE(port.hasData());

    // Второй peek возвращает то же самое
    auto p2 = port.peek();
    EXPECT_EQ(p2->id, 42);
}

TEST_F(PortTest, RecvConsumesData) {
    port.trySend(test_pkt);

    auto received = port.tryRecv();
    ASSERT_TRUE(received.has_value());
    EXPECT_EQ(received->id, 42);

    // После recv порт пуст
    EXPECT_FALSE(port.hasData());
    EXPECT_TRUE(port.isReady());
    EXPECT_FALSE(port.peek().has_value());
}

TEST_F(PortTest, RecvFromEmptyPort) {
    auto result = port.tryRecv();
    EXPECT_FALSE(result.has_value());
}

// === Тесты на копирование ===

TEST_F(PortTest, PacketIsCopiedNotMoved) {
    port.trySend(test_pkt);

    // Изменяем исходный пакет — это не должно повлиять на буфер
    test_pkt.id = 999;

    auto peeked = port.peek();
    EXPECT_EQ(peeked->id, 42); // Оригинальное значение
}
