/**
 * @file router_test.cpp
 * @brief Google Test для класса Router (интеграционные тесты).
 * @author Novoselov Alexander
 * @date 14/03/2026
 */

#include <gtest/gtest.h>
#include "../src/router.hpp"
#include "../src/packet.hpp"

/**
 * @class RouterTest
 * @brief Фикстура для тестов маршрутизатора.
 */
class RouterTest : public ::testing::Test {
protected:
    static constexpr int PORTS = 4;
    Router router{PORTS, PORTS, 42}; // 4x4, ID=42

    // Хелпер: инжект пакета во входной порт
    void inject(int port_idx, const Packet& pkt) {
        bool ok = router.getInputPort(port_idx).trySend(pkt);
        ASSERT_TRUE(ok) << "Failed to inject to port " << port_idx;
    }

    // Хелпер: проверить, что пакет доставлен на выход
    bool packetDelivered(int out_idx, int expected_id) {
        auto pkt = router.getOutputPort(out_idx).tryRecv();
        return pkt.has_value() && pkt->id == expected_id;
    }
};

// === Тесты инициализации ===

TEST_F(RouterTest, ConstructorInitializes) {
    EXPECT_EQ(router.getId(), 42);
    EXPECT_EQ(router.getInputCount(), 4);
    EXPECT_EQ(router.getOutputCount(), 4);

    // Все порты изначально пусты
    for (int i = 0; i < PORTS; ++i) {
        EXPECT_FALSE(router.getInputPort(i).hasData());
        EXPECT_FALSE(router.getOutputPort(i).hasData());
    }
}

// === Тесты базовой маршрутизации ===

TEST_F(RouterTest, SimplePassThrough) {
    // Заглушка route_pkt всегда возвращает 0
    Packet pkt{100, 10, 20};
    inject(0, pkt);

    router.on_clock();

    // Пакет должен появиться на выходе 0
    EXPECT_TRUE(packetDelivered(0, 100));
    // Вход 0 должен очиститься
    EXPECT_FALSE(router.getInputPort(0).hasData());
}

TEST_F(RouterTest, MultipleInputs_SameOutput) {
    // Два пакета на разных входах, оба хотят на выход 0 (заглушка)
    inject(0, Packet{1, 1, 1});
    inject(1, Packet{2, 2, 2});

    router.on_clock();

    // Ровно один пакет должен пройти (арбитраж)
    int delivered = 0;
    if (router.getOutputPort(0).tryRecv().has_value()) delivered++;

    EXPECT_EQ(delivered, 1);

    // Один вход должен остаться с пакетом (проигравший в арбитраже)
    int stalled = 0;
    if (router.getInputPort(0).hasData()) stalled++;
    if (router.getInputPort(1).hasData()) stalled++;

    EXPECT_EQ(stalled, 1);
}

// === Тесты Backpressure ===

TEST_F(RouterTest, Backpressure_PacketNotLost) {
    // Занимаем выход 0
    router.getOutputPort(0).trySend(Packet{999, 0, 0});
    ASSERT_TRUE(router.getOutputPort(0).hasData());

    // Пытаемся отправить пакет на вход 0 (заглушка ведет на выход 0)
    Packet important{777, 10, 20};
    inject(0, important);

    router.on_clock();

    // Пакет НЕ должен потеряться!
    EXPECT_TRUE(router.getInputPort(0).hasData());
    auto held = router.getInputPort(0).peek();
    ASSERT_TRUE(held.has_value());
    EXPECT_EQ(held->id, 777);

    // Выход 0 все еще занят блокером
    EXPECT_TRUE(router.getOutputPort(0).hasData());
}

TEST_F(RouterTest, Backpressure_Recovery) {
    // Сценарий: выход занят → освободился → пакет доставлен

    // 1. Занимаем выход
    router.getOutputPort(0).trySend(Packet{999, 0, 0});
    inject(0, Packet{111, 1, 2});

    // 2. Такт: пакет застрял на входе
    router.on_clock();
    EXPECT_TRUE(router.getInputPort(0).hasData());

    // 3. Освобождаем выход
    router.getOutputPort(0).tryRecv();
    ASSERT_FALSE(router.getOutputPort(0).hasData());

    // 4. Следующий такт: пакет должен пройти
    router.on_clock();

    EXPECT_TRUE(packetDelivered(0, 111));
    EXPECT_FALSE(router.getInputPort(0).hasData());
}

// === Тесты арбитража (интеграция с RRArbiter) ===

TEST_F(RouterTest, Arbitration_RoundRobin) {
    // Постоянные запросы от входов 0 и 1 на выход 0
    auto sendBoth = [this]() {
        if (!router.getInputPort(0).hasData())
            inject(0, Packet{1, 0, 0});
        if (!router.getInputPort(1).hasData())
            inject(1, Packet{2, 1, 0});
    };

    // Такт 1: должен выиграть вход 0 (голова арбитра = 0)
    sendBoth();
    router.on_clock();
    EXPECT_TRUE(packetDelivered(0, 1)); // Пакет от входа 0
    EXPECT_TRUE(router.getInputPort(1).hasData()); // Вход 1 ждет

    // Очищаем выход для следующего такта
    router.getOutputPort(0).tryRecv();

    // Такт 2: должен выиграть вход 1 (голова сдвинулась)
    sendBoth(); // Восстанавливаем пакет на входе 0
    router.on_clock();

    // Проверяем, что на выходе пакет от входа 1 (ид=2)
    auto out = router.getOutputPort(0).tryRecv();
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(out->id, 2);
}

// === Тесты метода peek в collect_requests ===

TEST_F(RouterTest, CollectDoesNotConsume) {
    // Проверяем, что фаза collect не извлекает пакеты
    Packet pkt{55, 1, 2};
    inject(0, pkt);

    // Можем вручную вызвать collect_requests (если сделать public для тестов)
    // Или проверяем через on_clock + backpressure:

    // Занимаем выход → пакет не должен уйти с входа
    router.getOutputPort(0).trySend(Packet{999, 0, 0});

    router.on_clock();

    // Пакет все еще на входе (collect использовал peek, а не tryRecv)
    EXPECT_TRUE(router.getInputPort(0).hasData());
    EXPECT_EQ(router.getInputPort(0).peek()->id, 55);
}
