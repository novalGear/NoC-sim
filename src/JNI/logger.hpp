#pragma once

#include <string>
#include <iostream>

namespace jni_logger {

enum class Level {
    INFO,
    WARNING,
    ERROR
};

inline void log(Level level, const std::string& message) {
    const char* prefix = "";
    switch (level) {
        case Level::WARNING: prefix = "[C++ WARN] "; break;
        case Level::ERROR:   prefix = "[C++ ERROR] "; break;
        default:             prefix = "[C++] "; break;
    }
    std::cout << prefix << message << std::endl;
}

inline void info(const std::string& msg)  { log(Level::INFO, msg); }
inline void warn(const std::string& msg)  { log(Level::WARNING, msg); }
inline void error(const std::string& msg) { log(Level::ERROR, msg); }

} // namespace jni_logger
