#include <jni.h>
#include <string>
#include <nlohmann/json.hpp>

#include "logger.hpp"
#include "json_utils.hpp"
#include "config_validator.hpp"
#include "simulation_runner.hpp"

using json = nlohmann::json;

// Thread-local состояние
thread_local static std::string g_last_error;
thread_local static json g_config_data;
thread_local static bool g_initialized = false;

// ============================================================
// JNI Implementation
// ============================================================

extern "C" {

JNIEXPORT jint JNICALL Java_Interface_service_NativeInterface_nativeInitialize
  (JNIEnv* env, jobject /* obj */, jstring jsonConfig) {

    std::string json_str = json_utils::jstring_to_string(env, jsonConfig);
    g_last_error.clear();
    g_initialized = false;

    try {
        g_config_data = json::parse(json_str);

        auto validation = config_validator::validate_all(g_config_data);
        if (!validation.valid) {
            g_last_error = validation.error_message;
            jni_logger::error("Config validation failed: " + validation.error_message);
            return 1;
        }

        config_validator::log_configuration(g_config_data);

        g_initialized = true;
        return 0;

    } catch (const json::parse_error& e) {
        g_last_error = std::string("JSON parse error: ") + e.what();
        jni_logger::error(g_last_error);
        return 2;
    } catch (const std::exception& e) {
        g_last_error = std::string("Initialization error: ") + e.what();
        jni_logger::error(g_last_error);
        return 3;
    } catch (...) {
        g_last_error = "Unknown error during initialization";
        jni_logger::error(g_last_error);
        return -1;
    }
}

JNIEXPORT jstring JNICALL Java_Interface_service_NativeInterface_nativeRun
  (JNIEnv* env, jobject /* obj */) {

    try {
        if (!g_initialized || g_config_data.is_null()) {
            json error = json_utils::make_error_response(
                "Simulation not initialized. Call nativeInitialize first."
            );
            std::string error_str = error.dump();
            return env->NewStringUTF(error_str.c_str());
        }

        json result = simulation_runner::run_simulation(g_config_data);

        std::string result_str = result.dump();
        return env->NewStringUTF(result_str.c_str());

    } catch (const std::exception& e) {
        json error = json_utils::make_error_response(
            std::string("Runtime error: ") + e.what()
        );
        std::string error_str = error.dump();
        return env->NewStringUTF(error_str.c_str());
    } catch (...) {
        json error = json_utils::make_error_response("Unknown runtime error");
        std::string error_str = error.dump();
        return env->NewStringUTF(error_str.c_str());
    }
}

JNIEXPORT jstring JNICALL Java_Interface_service_NativeInterface_nativeGetLastError
  (JNIEnv* env, jobject /* obj */) {
    return env->NewStringUTF(g_last_error.c_str());
}

} // extern "C"
