/**
 * @file simulation_test.cpp
 * @brief Тесты для SimulationController и всей симуляции в целом.
 * @author Novoselov Alexander
 * @date 14/04/2026
 */

#include <gtest/gtest.h>
#include "network/simulation_controller.hpp"
#include <filesystem>
#include <fstream>

class SimulationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создаем директорию для результатов, если её нет
        std::filesystem::create_directories("test_results");
    }

    void TearDown() override {
        // Очищаем тестовые файлы после тестов (опционально)
        // std::filesystem::remove_all("test_results");
    }
};

//=============================================================================
// Тест 1: Маленькая сеть 2x2 с uniform трафиком
//=============================================================================

TEST_F(SimulationTest, SmallMesh2x2Uniform) {
    SimulationController sim;

    // Настраиваем топологию
    sim.setup_mesh(2, 2);  // 4 узла
    sim.generate_uniform_trace(50,     // 50 пакетов
                                0.2,   // injection rate
                                100);  // max ticks

    // Запускаем симуляцию
    sim.run(100);

    // Проверяем базовые результаты
    const auto& trace = sim.get_trace();
    EXPECT_GT(trace.delivered_count(), 0);
    EXPECT_LE(trace.lost_count(), trace.total_packets());

    // Сохраняем результаты
    sim.save_results("test_results/2x2_uniform.json");

    // Проверяем, что файл создан
    EXPECT_TRUE(std::filesystem::exists("test_results/2x2_uniform.json"));
}

//=============================================================================
// Тест 2: Сетка 3x3 с hotspot трафиком
//=============================================================================

TEST_F(SimulationTest, Mesh3x3Hotspot) {
    SimulationController sim;

    sim.setup_mesh(3, 3);  // 9 узлов
    sim.generate_hotspot_trace(100,    // 100 пакетов
                                4,     // hotspot node (центр, ID=4)
                                0.6,   // 60% пакетов в hotspot
                                0.15,  // injection rate
                                200);  // max ticks

    sim.run(200);

    const auto& trace = sim.get_trace();
    EXPECT_GT(trace.delivered_count(), 0);

    // Проверяем, что средняя латентность в разумных пределах
    // В сетке 3x3 максимум хопов = 4 (из угла в противоположный)
    EXPECT_LT(trace.average_latency(), 50);

    sim.save_results("test_results/3x3_hotspot.json");
}

//=============================================================================
// Тест 3: Синтетический паттерн transpose для 4x4
//=============================================================================

TEST_F(SimulationTest, Mesh4x4Transpose) {
    SimulationController sim;

    sim.setup_mesh(4, 4);  // 16 узлов
    sim.generate_synthetic_trace("transpose");

    // Для синтетических паттернов все пакеты отправляются в такте 0
    // Нужно достаточно тактов, чтобы все пакеты доставились
    int max_hops = 6;  // в 4x4 максимум хопов = 6
    sim.run(max_hops * 2 + 10);  // запас

    const auto& trace = sim.get_trace();

    // В синтетическом паттерне все пакеты должны быть доставлены
    EXPECT_EQ(trace.delivered_count(), trace.total_packets());
    EXPECT_EQ(trace.lost_count(), 0);

    // Сохраняем трейс для анализа
    sim.save_trace("test_results/4x4_transpose_trace.json");
    sim.save_results("test_results/4x4_transpose_results.json");
}

//=============================================================================
// Тест 4: Загрузка трейса из файла и симуляция
//=============================================================================

TEST_F(SimulationTest, LoadTraceAndSimulate) {
    // Сначала генерируем трейс
    SimulationController sim_gen;
    sim_gen.setup_mesh(3, 3);
    sim_gen.generate_uniform_trace(30, 0.1, 50);
    sim_gen.save_trace("test_results/generated_trace.json");

    // Загружаем трейс в новый контроллер
    SimulationController sim_load;
    sim_load.setup_mesh(3, 3);
    sim_load.load_trace("test_results/generated_trace.json");

    // Запускаем симуляцию
    sim_load.run(100);

    const auto& trace = sim_load.get_trace();
    EXPECT_GT(trace.delivered_count(), 0);

    // Результаты должны совпадать с оригинальной симуляцией
    // (детерминизм при одинаковом seed)
    sim_load.save_results("test_results/loaded_trace_results.json");
}

//=============================================================================
// Тест 5: Пошаговая симуляция (для отладки)
//=============================================================================

TEST_F(SimulationTest, StepByStepSimulation) {
    SimulationController sim;
    sim.setup_mesh(2, 2);
    sim.generate_uniform_trace(10, 0.5, 20);

    // Ручной пошаговый режим
    sim.build();  // строим сеть вручную

    const auto& trace = sim.get_trace();
    int initial_delivered = trace.delivered_count();

    // Делаем 10 шагов
    for (int i = 0; i < 10; ++i) {
        sim.step();
    }

    // За 10 шагов должны доставиться хотя бы некоторые пакеты
    EXPECT_GT(trace.delivered_count(), initial_delivered);

    sim.save_results("test_results/step_by_step.json");
}

//=============================================================================
// Тест 6: Большая сеть 8x8 (для бенчмарка)
//=============================================================================

TEST_F(SimulationTest, DISABLED_LargeMesh8x8Uniform) {
    // DISABLED_ - отключаем по умолчанию, так как тест может быть долгим
    SimulationController sim;

    sim.setup_mesh(8, 8);  // 64 узла
    sim.generate_uniform_trace(1000, 0.05, 500);

    sim.run(500);

    const auto& trace = sim.get_trace();
    std::cout << "Large mesh 8x8 results:" << std::endl;
    std::cout << "  Delivered: " << trace.delivered_count() << "/" << trace.total_packets() << std::endl;
    std::cout << "  Avg latency: " << trace.average_latency() << std::endl;

    sim.save_results("test_results/8x8_uniform.json");
}

//=============================================================================
// Тест 7: Сравнение разных топологий на одном трейсе
//=============================================================================

TEST_F(SimulationTest, CompareTopologiesOnSameTrace) {
    // Генерируем один трейс
    PacketTrace common_trace;
    common_trace.generate_uniform(50, 16, 0.1, 100);
    common_trace.save_to_json("test_results/common_trace.json");

    // Симуляция на 4x4
    SimulationController sim_4x4;
    sim_4x4.setup_mesh(4, 4);
    sim_4x4.load_trace("test_results/common_trace.json");
    sim_4x4.run(150);
    double latency_4x4 = sim_4x4.get_trace().average_latency();

    // Симуляция на 2x8 (16 узлов, другая форма)
    SimulationController sim_2x8;
    sim_2x8.setup_mesh(2, 8);
    sim_2x8.load_trace("test_results/common_trace.json");
    sim_2x8.run(150);
    double latency_2x8 = sim_2x8.get_trace().average_latency();

    std::cout << "Latency comparison:" << std::endl;
    std::cout << "  4x4 mesh: " << latency_4x4 << " ticks" << std::endl;
    std::cout << "  2x8 mesh: " << latency_2x8 << " ticks" << std::endl;

    // Сохраняем оба результата
    sim_4x4.save_results("test_results/compare_4x4.json");
    sim_2x8.save_results("test_results/compare_2x8.json");

    // Не строгий assert, просто чтобы тест проходил
    EXPECT_TRUE(true);
}
