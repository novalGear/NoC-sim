#pragma once

#include <jni.h>
#include <string>
#include <nlohmann/json.hpp>

namespace json_utils {

using json = nlohmann::json;

// Конвертация jstring в std::string
std::string jstring_to_string(JNIEnv* env, jstring jstr);

// Создание JSON с ошибкой
json make_error_response(const std::string& error_msg);

// Создание JSON с успешным результатом
json make_success_response();

// Извлечение строки из JSON с проверкой
std::string get_string(const json& j, const std::string& key,
                       const std::string& default_val = "");

// Извлечение числа из JSON с проверкой
int get_int(const json& j, const std::string& key, int default_val = 0);
double get_double(const json& j, const std::string& key, double default_val = 0.0);

} // namespace json_utils
