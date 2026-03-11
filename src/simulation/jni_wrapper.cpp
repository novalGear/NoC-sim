#include <jni.h>
#include <string>
#include <iostream>
// #include <nlohmann/json.hpp> // Если будете использовать этот парсер

static std::string g_lastError;

// Хелпер для конвертации
std::string jstringToStdString(JNIEnv* env, jstring jStr) {
    if (!jStr) return "";
    const char* cStr = env->GetStringUTFChars(jStr, nullptr);
    std::string result(cStr);
    env->ReleaseStringUTFChars(jStr, cStr);
    return result;
}

// Новая сигнатура: принимаем String из Java
extern "C" JNIEXPORT jint JNICALL
Java_InterconnectGUI_nativeInitialize(JNIEnv* env, jobject obj, jstring jsonConfig) {

    std::string config = jstringToStdString(env, jsonConfig);
    g_lastError.clear();

    // TODO: Здесь будет парсинг JSON и инициализация симуляции
    // Пока просто логируем для проверки связи
    std::cout << "[C++] Config received: " << config << std::endl;

    // Пример проверки (позже замените на реальную валидацию)
    if (config.find("\"width\":0") != std::string::npos) {
        g_lastError = "Width cannot be zero";
        return 1; // Код ошибки
    }

    return 0; // Успех
}

extern "C" JNIEXPORT jint JNICALL
Java_InterconnectGUI_nativeRun(JNIEnv* env, jobject obj) {
    std::cout << "[C++] Running simulation..." << std::endl;
    // TODO: Запуск реальной симуляции
    return 0;
}

extern "C" JNIEXPORT jstring JNICALL
Java_InterconnectGUI_nativeGetLastError(JNIEnv* env, jobject obj) {
    return env->NewStringUTF(g_lastError.c_str());
}
