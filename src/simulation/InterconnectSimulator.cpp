#include <jni.h>
#include <string>
#include <sstream>
#include <chrono>
#include "InterconnectConfig.h"

TopologyType parseTopology(const std::string& str) {
    if (str == "MESH") return TopologyType::MESH;
    if (str == "BUTTERFLY") return TopologyType::BUTTERFLY;
    return TopologyType::UNKNOWN;
}

RoutingType parseRouting(const std::string& str) {
    if (str == "OBLIVIOUS") return RoutingType::OBLIVIOUS;
    if (str == "ADAPTIVE") return RoutingType::ADAPTIVE;
    return RoutingType::UNKNOWN;
}

SimulationResult runSimulationCpp(const InterconnectConfig& config) {
    SimulationResult result;
    auto start = std::chrono::high_resolution_clock::now();

    if (!config.isValid()) {
        result.success = false;
        result.errorMessage = "Invalid configuration parameters";
        return result;
    }

    // ... ЗДЕСЬ ВАША РЕАЛЬНАЯ ЛОГИКА СИМУЛЯЦИИ ...
    // Например:
    // Network network(config);
    // network.run();
    // result.latency_avg = network.getAvgLatency();

    // Заглушка для примера
    result.success = true;
    result.latency_avg = 12.5;
    result.throughput = 1000.0;
    result.packets_delivered = config.nodes * 10;

    auto end = std::chrono::high_resolution_clock::now();
    result.execution_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    return result;
}

// --- JNI Функция (Мост между Java и C++) ---
extern "C" JNIEXPORT jstring JNICALL
Java_javajni_JavaJNI_runNativeSimulation
  (JNIEnv *env, jobject, jstring configStr) {

    // 1. Получаем строку из Java
    const char *configChars = env->GetStringUTFChars(configStr, 0);
    std::string configData(configChars);
    env->ReleaseStringUTFChars(configStr, configChars);

    // 2. Парсим строку в структуру
    // Ожидаемый формат: "width;height;nodes;topology;routing"
    // Пример: "8;8;64;MESH;OBLIVIOUS"
    InterconnectConfig config;
    std::stringstream ss(configData);
    std::string token;
    int field = 0;

    while (std::getline(ss, token, ';')) {
        try {
            switch(field) {
                case 0: config.width = std::stoi(token); break;
                case 1: config.height = std::stoi(token); break;
                case 2: config.nodes = std::stoi(token); break;
                case 3: config.topology = parseTopology(token); break;
                case 4: config.routing = parseRouting(token); break;
            }
            field++;
        } catch (...) {
            // Ошибка парсинга
        }
    }

    // 3. Запускаем симуляцию
    SimulationResult result = runSimulationCpp(config);

    // 4. Формируем строку результата для Java
    // Формат: "success;latency;throughput;delivered;lost;time_ms;error"
    std::ostringstream resultStream;
    resultStream << (result.success ? "true" : "false") << ";"
                 << result.latency_avg << ";"
                 << result.throughput << ";"
                 << result.packets_delivered << ";"
                 << result.packets_lost << ";"
                 << result.execution_time_ms << ";"
                 << result.errorMessage;

    return env->NewStringUTF(resultStream.str().c_str());
}
