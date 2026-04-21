#include "json_utils.hpp"
#include "logger.hpp"

namespace json_utils {

std::string jstring_to_string(JNIEnv* env, jstring jstr) {
    if (!jstr) return "";
    const char* cstr = env->GetStringUTFChars(jstr, nullptr);
    std::string result(cstr);
    env->ReleaseStringUTFChars(jstr, cstr);
    return result;
}

json make_error_response(const std::string& error_msg) {
    json response;
    response["success"] = false;
    response["error"] = error_msg;
    return response;
}

json make_success_response() {
    json response;
    response["success"] = true;
    response["error"] = "";
    return response;
}

std::string get_string(const json& j, const std::string& key,
                       const std::string& default_val) {
    if (j.contains(key) && j[key].is_string()) {
        return j[key].get<std::string>();
    }
    return default_val;
}

int get_int(const json& j, const std::string& key, int default_val) {
    if (j.contains(key) && j[key].is_number()) {
        return j[key].get<int>();
    }
    return default_val;
}

double get_double(const json& j, const std::string& key, double default_val) {
    if (j.contains(key) && j[key].is_number()) {
        return j[key].get<double>();
    }
    return default_val;
}

} // namespace json_utils
