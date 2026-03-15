/**
 * @file rr_arbiter_test.cpp
 * @brief Google Test для класса RRArbiter (Round-Robin Arbiter).
 * @author Novoselov Alexander
 * @date 14/03/2026
 */

#include <gtest/gtest.h>
#include "../src/rr_arbiter.hpp"
#include "../src/packet.hpp"

/**
 * @class RRArbiterTest
 * @brief Фикстура для тестов арбитра.
 */
class RRArbiterTest : public ::testing::Test {
protected:
    static constexpr int PORT_COUNT = 4;
    RRArbiter arbiter{PORT_COUNT};

    // Хелпер: создать запрос от порта
    Request makeRequest(int port_idx) {
        return Request{port_idx};
    }
};

// === Базовые тесты ===

TEST_F(RRArbiterTest, EmptyRequestList) {
    std::vector<Request> empty;
    EXPECT_EQ(arbiter.arbitrate(empty, 0), -1);
}

TEST_F(RRArbiterTest, SingleRequestWins) {
    std::vector<Request> reqs = {makeRequest(2)};
    int winner = arbiter.arbitrate(reqs, 0);

    EXPECT_EQ(winner, 0); // Индекс в векторе
    EXPECT_EQ(reqs[winner].src_port_idx, 2);
}

// === Тесты алгоритма Round-Robin ===

TEST_F(RRArbiterTest, RoundRobin_CyclicOrder) {
    // Голова начинает с 0
    // Запросы от всех портов: должен выиграть ближайший к голове (0)
    std::vector<Request> reqs = {
        makeRequest(0), makeRequest(1),
        makeRequest(2), makeRequest(3)
    };

    // Такт 1: голова=0 → побеждает порт 0
    int w1 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w1].src_port_idx, 0);

    // Такт 2: голова=1 → побеждает порт 1
    int w2 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w2].src_port_idx, 1);

    // Такт 3: голова=2 → побеждает порт 2
    int w3 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w3].src_port_idx, 2);

    // Такт 4: голова=3 → побеждает порт 3
    int w4 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w4].src_port_idx, 3);

    // Такт 5: голова=0 (цикл замкнулся) → снова порт 0
    int w5 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w5].src_port_idx, 0);
}

TEST_F(RRArbiterTest, RoundRobin_SkipsNonRequesting) {
    // Запрошены только порты 1 и 3, голова = 0
    std::vector<Request> reqs = {makeRequest(1), makeRequest(3)};

    // dist(1) = (1+4-0)%4 = 1
    // dist(3) = (3+4-0)%4 = 3
    // Побеждает порт 1 (меньшее расстояние)
    int w1 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w1].src_port_idx, 1);

    // Теперь голова = 2
    // dist(1) = (1+4-2)%4 = 3
    // dist(3) = (3+4-2)%4 = 1
    // Побеждает порт 3
    int w2 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w2].src_port_idx, 3);
}

TEST_F(RRArbiterTest, NoStarvation) {
    // Проверка, что порт не голодает при постоянных запросах от других
    // Запрошены порты 0 и 2, голова = 0
    std::vector<Request> reqs = {makeRequest(0), makeRequest(2)};

    // Порт 0 выигрывает первый раз (dist=0)
    int w1 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w1].src_port_idx, 0);

    // Голова сдвинулась на 1. Теперь:
    // dist(0) = 3, dist(2) = 1 → побеждает порт 2
    int w2 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w2].src_port_idx, 2);

    // Голова = 3.
    // dist(0) = 1, dist(2) = 3 → снова побеждает порт 0
    int w3 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w3].src_port_idx, 0);

    // Порт 0 не голодает, хотя порт 2 тоже постоянно запрашивает
}

// === Тесты параметра outputPortId ===

TEST_F(RRArbiterTest, OutputPortIdIgnored) {
    // outputPortId не влияет на логику, только для отладки
    std::vector<Request> reqs = {makeRequest(1)};

    int r1 = arbiter.arbitrate(reqs, 0);
    int r2 = arbiter.arbitrate(reqs, 999);

    // Результат должен быть одинаковым
    EXPECT_EQ(r1, r2);
    EXPECT_EQ(reqs[r1].src_port_idx, 1);
}
