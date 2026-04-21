#include <jni.h>
#include <string>
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <atomic>

// Если используешь nlohmann/json
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Если нет nlohmann, можно использовать свой парсер
// #include "JsonParser.hpp"

// Thread-local хранение состояния
thread_local static std::string g_lastError;
thread_local static json g_configData;
thread_local static bool g_initialized = false;

// Вспомогательная функция: jstring -> std::string
std::string jstringToString(JNIEnv* env, jstring jStr) {
    if (!jStr) return "";
    const char* cStr = env->GetStringUTFChars(jStr, nullptr);
    std::string result(cStr);
    env->ReleaseStringUTFChars(jStr, cStr);
    return result;
}

// Вспомогательная функция для проверки конфигурации
bool validateConfig(const json& config, std::string& errorMsg) {
    // Проверка наличия обязательных полей
    if (!config.contains("topology") || !config.contains("routing") ||
        !config.contains("width") || !config.contains("height") ||
        !config.contains("nodes")) {
        errorMsg = "Missing required fields in configuration";
        return false;
    }

    // Проверка типов и значений
    try {
        std::string topology = config["topology"].get<std::string>();
        std::string routing = config["routing"].get<std::string>();
        int width = config["width"].get<int>();
        int height = config["height"].get<int>();
        int nodes = config["nodes"].get<int>();

        if (topology != "MESH" && topology != "BUTTERFLY") {
            errorMsg = "Invalid topology type: " + topology;
            return false;
        }

        if (routing != "OBLIVIOUS" && routing != "ADAPTIVE") {
            errorMsg = "Invalid routing type: " + routing;
            return false;
        }

        if (width <= 0 || width > 64) {
            errorMsg = "Width must be between 1 and 64";
            return false;
        }

        if (height <= 0 || height > 64) {
            errorMsg = "Height must be between 1 and 64";
            return false;
        }

        if (nodes <= 0 || nodes > 4096) {
            errorMsg = "Nodes must be between 1 and 4096";
            return false;
        }

        // Дополнительная проверка для Mesh
        if (topology == "MESH" && nodes != width * height) {
            errorMsg = "For MESH topology, nodes must equal width * height";
            return false;
        }

    } catch (const std::exception& e) {
        errorMsg = std::string("Config validation error: ") + e.what();
        return false;
    }

    return true;
}

// Логика симуляции Mesh
json simulateMesh(const json& config) {
    json result;

    int width = config["width"];
    int height = config["height"];
    int nodes = config["nodes"];
    std::string routing = config["routing"];

    // Здесь будет реальная симуляция
    // Пока заглушка с более реалистичными данными

    // Имитация времени выполнения
    auto start = std::chrono::high_resolution_clock::now();

    // "Симуляция" - небольшая задержка для реалистичности
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Генерация результатов на основе параметров
    int packetsTotal = nodes * 100;  // 100 пакетов на узел

    // Моделируем потери в зависимости от размера сети
    double lossRate = 0.01 + (width * height / 1000.0);
    if (routing == "ADAPTIVE") {
        lossRate *= 0.7;  // Адаптивная маршрутизация уменьшает потери
    }

    int packetsLost = static_cast<int>(packetsTotal * lossRate);
    int packetsDelivered = packetsTotal - packetsLost;

    // Задержка зависит от размера сети
    double baseLatency = 10.0;
    double latencyFactor = (width + height) * 0.5;
    double latencyAvg = baseLatency + latencyFactor;
    double latencyMax = latencyAvg * 2.5;

    // Пропускная способность
    double throughput = packetsDelivered / (latencyAvg * nodes * 0.1);

    auto end = std::chrono::high_resolution_clock::now();
    long long execTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Заполнение результата
    result["success"] = true;
    result["latency_avg"] = latencyAvg;
    result["latency_max"] = latencyMax;
    result["throughput"] = throughput;
    result["packets_sent"] = packetsTotal;
    result["packets_delivered"] = packetsDelivered;
    result["packets_lost"] = packetsLost;
    result["cycles_elapsed"] = static_cast<long long>(latencyAvg * packetsTotal);
    result["execution_time_ms"] = execTime;
    result["error"] = "";

    return result;
}

// Симуляция Butterfly (заглушка)
json simulateButterfly(const json& config) {
    json result;

    // Аналогично, но с другими формулами
    result["success"] = true;
    result["latency_avg"] = 15.0;
    result["latency_max"] = 30.0;
    result["throughput"] = 800.0;
    result["packets_sent"] = 1000;
    result["packets_delivered"] = 950;
    result["packets_lost"] = 50;
    result["cycles_elapsed"] = 15000;
    result["execution_time_ms"] = 50;
    result["error"] = "";

    return result;
}

// Основная логика симуляции
json runSimulationLogic(const json& config) {
    std::string topology = config["topology"];

    if (topology == "MESH") {
        return simulateMesh(config);
    } else if (topology == "BUTTERFLY") {
        return simulateButterfly(config);
    } else {
        json error;
        error["success"] = false;
        error["error"] = "Unsupported topology: " + topology;
        return error;
    }
}

// ============= JNI Реализация =============

extern "C" {

JNIEXPORT jint JNICALL Java_Interface_service_NativeInterface_nativeInitialize
  (JNIEnv* env, jobject obj, jstring jsonConfig) {

    std::string jsonStr = jstringToString(env, jsonConfig);
    g_lastError.clear();
    g_initialized = false;

    try {
        // Парсинг JSON
        g_configData = json::parse(jsonStr);

        // Валидация
        std::string errorMsg;
        if (!validateConfig(g_configData, errorMsg)) {
            g_lastError = errorMsg;
            return 1;
        }

        std::cout << "[C++] Configuration loaded successfully" << std::endl;
        std::cout << "  Topology: " << g_configData["topology"] << std::endl;
        std::cout << "  Routing: " << g_configData["routing"] << std::endl;
        std::cout << "  Size: " << g_configData["width"] << "x" << g_configData["height"] << std::endl;
        std::cout << "  Nodes: " << g_configData["nodes"] << std::endl;

        g_initialized = true;
        return 0;

    } catch (const json::parse_error& e) {
        g_lastError = std::string("JSON parse error: ") + e.what();
        return 2;
    } catch (const std::exception& e) {
        g_lastError = std::string("Initialization error: ") + e.what();
        return 3;
    } catch (...) {
        g_lastError = "Unknown error during initialization";
        return -1;
    }
}

JNIEXPORT jstring JNICALL Java_Interface_service_NativeInterface_nativeRun
  (JNIEnv* env, jobject obj) {

    try {
        if (!g_initialized || g_configData.is_null()) {
            json error;
            error["success"] = false;
            error["error"] = "Simulation not initialized. Call nativeInitialize first.";
            std::string errorStr = error.dump();
            return env->NewStringUTF(errorStr.c_str());
        }

        std::cout << "[C++] Running simulation..." << std::endl;

        // Запуск симуляции
        json result = runSimulationLogic(g_configData);

        // Конвертация в строку
        std::string resultStr = result.dump();
        std::cout << "[C++] Simulation completed" << std::endl;

        return env->NewStringUTF(resultStr.c_str());

    } catch (const std::exception& e) {
        json error;
        error["success"] = false;
        error["error"] = std::string("Runtime error: ") + e.what();
        std::string errorStr = error.dump();
        return env->NewStringUTF(errorStr.c_str());
    } catch (...) {
        json error;
        error["success"] = false;
        error["error"] = "Unknown runtime error";
        std::string errorStr = error.dump();
        return env->NewStringUTF(errorStr.c_str());
    }
}

JNIEXPORT jstring JNICALL Java_Interface_service_NativeInterface_nativeGetLastError
  (JNIEnv* env, jobject obj) {
    return env->NewStringUTF(g_lastError.c_str());
}

} // extern "C"
