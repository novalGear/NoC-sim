/**
@file simulation_test.cpp
@brief Tests for SimulationController.
@author Novoselov Alexander
@date 14/04/2026
*/
#include <gtest/gtest.h>
#include "network/simulation_controller.hpp"
#include "rr_arbiter.hpp"
#include <filesystem>

class SimulationTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::filesystem::create_directories("test_results");
    }
};

//=============================================================================
// Тест 1: Маленькая сеть 2x2 с uniform трафиком
//=============================================================================
TEST_F(SimulationTest, SmallMesh2x2Uniform) {
    // If SimulationController is templated:
    SimulationController<RRArbiter> sim;

    sim.setup_mesh(2, 2);
    sim.generate_uniform_trace(50, 0.2, 100);
    sim.run(100);

    const auto& trace = sim.get_trace();
    EXPECT_GT(trace.delivered_count(), 0);
    EXPECT_LE(trace.lost_count(), trace.total_packets());

    sim.save_results("test_results/2x2_uniform.json");
    EXPECT_TRUE(std::filesystem::exists("test_results/2x2_uniform.json"));
}

//=============================================================================
// Тест 2: Сетка 3x3 с hotspot трафиком
//=============================================================================
TEST_F(SimulationTest, Mesh3x3Hotspot) {
    SimulationController<RRArbiter> sim;
    sim.setup_mesh(3, 3);
    sim.generate_hotspot_trace(100, 4, 0.6, 0.15, 200);

    sim.run(200);

    const auto& trace = sim.get_trace();
    EXPECT_GT(trace.delivered_count(), 0);
    EXPECT_LT(trace.average_latency(), 50);

    sim.save_results("test_results/3x3_hotspot.json");
}

//=============================================================================
// Тест 3: Синтетический паттерн transpose для 4x4
//=============================================================================
TEST_F(SimulationTest, Mesh4x4Transpose) {
    SimulationController<RRArbiter> sim;
    sim.setup_mesh(4, 4);
    sim.generate_synthetic_trace("transpose");

    int max_hops = 6;
    sim.run(max_hops * 2 + 10);

    const auto& trace = sim.get_trace();
    EXPECT_EQ(trace.delivered_count(), trace.total_packets());
    EXPECT_EQ(trace.lost_count(), 0);

    sim.save_trace("test_results/4x4_transpose_trace.json");
    sim.save_results("test_results/4x4_transpose_results.json");
}

//=============================================================================
// Тест 4: Загрузка трейса из файла и симуляция
//=============================================================================
TEST_F(SimulationTest, LoadTraceAndSimulate) {
    SimulationController<RRArbiter> sim_gen;
    sim_gen.setup_mesh(3, 3);
    sim_gen.generate_uniform_trace(30, 0.1, 50);
    sim_gen.save_trace("test_results/generated_trace.json");

    SimulationController<RRArbiter> sim_load;
    sim_load.setup_mesh(3, 3);
    sim_load.load_trace("test_results/generated_trace.json");

    sim_load.run(100);

    const auto& trace = sim_load.get_trace();
    EXPECT_GT(trace.delivered_count(), 0);

    sim_load.save_results("test_results/loaded_trace_results.json");
}

//=============================================================================
// Тест 5: Пошаговая симуляция
//=============================================================================
TEST_F(SimulationTest, StepByStepSimulation) {
    SimulationController<RRArbiter> sim;
    sim.setup_mesh(2, 2);
    sim.generate_uniform_trace(10, 0.5, 20);
    sim.build();

    const auto& trace = sim.get_trace();
    int initial_delivered = trace.delivered_count();

    for (int i = 0; i < 10; ++i) {
        sim.step();
    }

    EXPECT_GT(trace.delivered_count(), initial_delivered);
    sim.save_results("test_results/step_by_step.json");
}
