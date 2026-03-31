/**
 * @file rr_arbiter_test.cpp
 * @brief Google Test для класса RRArbiter (Round-Robin Arbiter).
 * @author Novoselov Alexander
 * @date 14/03/2026
 */

#include <gtest/gtest.h>
#include "rr_arbiter.hpp"
#include "packet.hpp"

class RRArbiterTest : public ::testing::Test {
protected:
    static constexpr int PORT_COUNT = 4;
    RRArbiter arbiter{PORT_COUNT};

    Request make_request(int port_idx) {
        return Request{port_idx};
    }
};

// === Базовые тесты ===

TEST_F(RRArbiterTest, EmptyRequestList) {
    std::vector<Request> empty;
    EXPECT_EQ(arbiter.arbitrate(empty, 0), -1);
}

TEST_F(RRArbiterTest, SingleRequestWins) {
    std::vector<Request> reqs = {make_request(2)};
    int winner = arbiter.arbitrate(reqs, 0);

    EXPECT_EQ(winner, 0);
    EXPECT_EQ(reqs[winner].src, 2);
}

// === Тесты алгоритма Round-Robin ===

TEST_F(RRArbiterTest, RoundRobin_CyclicOrder) {
    std::vector<Request> reqs = {
        make_request(0), make_request(1),
        make_request(2), make_request(3)
    };

    int w1 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w1].src, 0);

    int w2 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w2].src, 1);

    int w3 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w3].src, 2);

    int w4 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w4].src, 3);

    int w5 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w5].src, 0);
}

TEST_F(RRArbiterTest, RoundRobin_SkipsNonRequesting) {
    std::vector<Request> reqs = {make_request(1), make_request(3)};

    int w1 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w1].src, 1);

    int w2 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w2].src, 3);
}

TEST_F(RRArbiterTest, RoundRobin_ComplexPattern) {
    std::vector<Request> reqs = {make_request(0), make_request(2), make_request(3)};

    int w1 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w1].src, 0);

    int w2 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w2].src, 2);

    int w3 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w3].src, 3);

    int w4 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w4].src, 0);
}

TEST_F(RRArbiterTest, NoStarvation) {
    std::vector<Request> reqs = {make_request(0), make_request(2)};

    int w1 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w1].src, 0);

    int w2 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w2].src, 2);

    int w3 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w3].src, 0);
}

TEST_F(RRArbiterTest, AllPortsRequesting_Rotation) {
    std::vector<Request> all_ports = {
        make_request(0), make_request(1), make_request(2), make_request(3)
    };

    std::vector<int> winners;
    for (int i = 0; i < 8; ++i) {
        int winner_idx = arbiter.arbitrate(all_ports, 0);
        winners.push_back(all_ports[winner_idx].src);
    }

    std::vector<int> expected = {0, 1, 2, 3, 0, 1, 2, 3};
    EXPECT_EQ(winners, expected);
}

// === Тесты параметра outputPortId ===

TEST_F(RRArbiterTest, OutputPortIdIgnored) {
    std::vector<Request> reqs = {make_request(1)};

    int r1 = arbiter.arbitrate(reqs, 0);
    int r2 = arbiter.arbitrate(reqs, 999);
    int r3 = arbiter.arbitrate(reqs, -1);
    int r4 = arbiter.arbitrate(reqs, 42);

    EXPECT_EQ(r1, r2);
    EXPECT_EQ(r1, r3);
    EXPECT_EQ(r1, r4);
    EXPECT_EQ(reqs[r1].src, 1);
}

// === Тесты граничных условий ===

TEST_F(RRArbiterTest, SinglePortMultipleRequests) {
    std::vector<Request> reqs = {make_request(0), make_request(0), make_request(0)};

    int winner = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(winner, 0);
    EXPECT_EQ(reqs[winner].src, 0);
}

TEST_F(RRArbiterTest, EdgePorts) {
    std::vector<Request> reqs = {make_request(0), make_request(3)};

    int w1 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w1].src, 0);

    int w2 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w2].src, 3);

    int w3 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w3].src, 0);
}

TEST_F(RRArbiterTest, MaximumPortCount) {
    const int MAX_PORTS = 100;
    RRArbiter big_arbiter(MAX_PORTS);

    std::vector<Request> reqs;
    for (int i = 0; i < MAX_PORTS; ++i) {
        reqs.push_back(make_request(i));
    }

    for (int cycle = 0; cycle < 3; ++cycle) {
        for (int expected_port = 0; expected_port < MAX_PORTS; ++expected_port) {
            int winner_idx = big_arbiter.arbitrate(reqs, 0);
            EXPECT_EQ(reqs[winner_idx].src, expected_port);
        }
    }
}

// === Тесты последовательных вызовов ===

TEST_F(RRArbiterTest, SequentialCalls_MaintainState) {
    std::vector<Request> reqs = {make_request(0), make_request(1)};

    int w1 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w1].src, 0);

    int w2 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w2].src, 1);

    int w3 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w3].src, 0);

    int w4 = arbiter.arbitrate(reqs, 0);
    EXPECT_EQ(reqs[w4].src, 1);
}

TEST_F(RRArbiterTest, MixedRequestSets) {
    std::vector<Request> reqs_0_2 = {make_request(0), make_request(2)};
    std::vector<Request> reqs_1_3 = {make_request(1), make_request(3)};

    int w1 = arbiter.arbitrate(reqs_0_2, 0);
    EXPECT_EQ(reqs_0_2[w1].src, 0);

    int w2 = arbiter.arbitrate(reqs_1_3, 0);
    EXPECT_EQ(reqs_1_3[w2].src, 1);

    int w3 = arbiter.arbitrate(reqs_0_2, 0);
    EXPECT_EQ(reqs_0_2[w3].src, 2);

    int w4 = arbiter.arbitrate(reqs_1_3, 0);
    EXPECT_EQ(reqs_1_3[w4].src, 3);
}

// === Тесты на производительность ===

TEST_F(RRArbiterTest, ManyArbitrations) {
    std::vector<Request> reqs = {make_request(0), make_request(1), make_request(2)};

    for (int i = 0; i < 1000; ++i) {
        int winner = arbiter.arbitrate(reqs, 0);
        EXPECT_GE(winner, 0);
        EXPECT_LT(winner, static_cast<int>(reqs.size()));
    }
}

// === Тесты с разными outputPortId ===

TEST_F(RRArbiterTest, DifferentOutputPortsIndependent) {
    std::vector<Request> reqs = {make_request(0), make_request(1)};

    EXPECT_EQ(reqs[arbiter.arbitrate(reqs, 0)].src, 0);
    EXPECT_EQ(reqs[arbiter.arbitrate(reqs, 1)].src, 1);
    EXPECT_EQ(reqs[arbiter.arbitrate(reqs, 2)].src, 0);
    EXPECT_EQ(reqs[arbiter.arbitrate(reqs, 3)].src, 1);
    EXPECT_EQ(reqs[arbiter.arbitrate(reqs, 4)].src, 0);
}
