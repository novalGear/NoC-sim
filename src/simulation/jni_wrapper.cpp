#include <jni.h>
#include <string>
#include <iostream>
#include <nlohmann/json.hpp> // Подключение библиотеки

using json = nlohmann::json;

// Thread-local хранение состояния
thread_local static std::string g_lastError;
thread_local static json g_configData; // Храним конфиг как JSON объект

// Хелпер: jstring -> std::string
std::string jstringToStdString(JNIEnv* env, jstring jStr) {
    if (!jStr) return "";
    const char* cStr = env->GetStringUTFChars(jStr, nullptr);
    std::string result(cStr);
    env->ReleaseStringUTFChars(jStr, cStr);
    return result;
}

// JNI: Инициализация (принимает JSON строку)
extern "C" JNIEXPORT jint JNICALL
Java_InterconnectGUI_nativeInitialize(JNIEnv* env, jobject, jstring jsonConfig) {

    std::string jsonStr = jstringToStdString(env, jsonConfig);
    g_lastError.clear();

    try {
        // Парсим JSON используя nlohmann
        g_configData = json::parse(jsonStr);

        // Валидация полей
        if (!g_configData.contains("width") || !g_configData.contains("topology")) {
            throw std::runtime_error("Missing required fields");
        }

        int width = g_configData["width"].get<int>();
        std::string topo = g_configData["topology"].get<std::string>();

        if (width <= 0) {
            throw std::runtime_error("Width must be positive");
        }

        std::cout << "[C++] Config loaded: " << topo << " (" << width << "x" << g_configData["height"] << ")" << std::endl;

    } catch (json::exception& e) {
        g_lastError = std::string("JSON Parse Error: ") + e.what();
        return 1;
    } catch (std::exception& e) {
        g_lastError = e.what();
        return 2;
    } catch (...) {
        g_lastError = "Unknown error during initialization";
        return -1;
    }

    return 0; // Success
}

// Заглушка симуляции (возвращает JSON объект)
json runSimulationLogic(const json& config) {
    json result;

    // Имитация работы
    int nodes = config.value("nodes", 100);
    bool success = true;

    result["success"] = success;
    result["latency_avg"] = 12.5 + (config["width"].get<int>() * 0.1);
    result["latency_max"] = 45.0;
    result["throughput"] = static_cast<double>(nodes) * 0.95;
    result["packets_sent"] = nodes * 10;
    result["packets_delivered"] = static_cast<int>(nodes * 9.5);
    result["packets_lost"] = static_cast<int>(nodes * 0.5);
    result["cycles_elapsed"] = 1000;
    result["error"] = "";

    return result;
}

// JNI: Запуск (возвращает JSON строку)
extern "C" JNIEXPORT jstring JNICALL
Java_InterconnectGUI_nativeRun(JNIEnv* env, jobject) {

    try {
        if (g_configData.is_null()) {
            json err;
            err["success"] = false;
            err["error"] = "Simulation not initialized. Call nativeInitialize first.";
            return env->NewStringUTF(err.dump().c_str());
        }

        // Запуск логики
        json resultJson = runSimulationLogic(g_configData);

        // Конвертация JSON объекта в строку
        std::string resultStr = resultJson.dump();
        return env->NewStringUTF(resultStr.c_str());

    } catch (std::exception& e) {
        json err;
        err["success"] = false;
        err["error"] = std::string("Runtime Error: ") + e.what();
        return env->NewStringUTF(err.dump().c_str());
    }
}

// JNI: Получение ошибки
extern "C" JNIEXPORT jstring JNICALL
Java_InterconnectGUI_nativeGetLastError(JNIEnv* env, jobject) {
    return env->NewStringUTF(g_lastError.c_str());
}
