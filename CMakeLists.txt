# =========================================================
# Network-on-Crystal Simulation Root
# =========================================================

# 1. Минимальная версия и имя проекта
cmake_minimum_required(VERSION 3.10)
project(NoC_Simulatior_JNI CXX)

# 2. Глобальные настройки компилятора
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDART_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 3. Поиск внешних зависимостей
# Java
find_package(JAVA REQUIRED)
include(UseJava)    # вспомогательные функции для работы с Java

# jni.h
find_package(JNI REQUIRED)

# Google Tests
find_package(GTest REQUIRED)
include(GoogleTest) # для функции gtest_discover_tests

# nlohmann/json lib for C++
find_package(nlomann_json QUIET)

message(STATUS "Java found: ${Java_JAVA_EXECUTABLE}")
message(STATUS "JNI include dirs: ${JNI_INCLUDE_DIRS}")

# 4. Определение путей и переменных проекта

set(JSON_JAR "${CMAKE_SOURCE_DIR}/lib/java/json-20231013.jar")

# Папка для всех артефактов сборки (классы, библиотеки)
set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/output")
set(CLASS_DIR "${OUTPUT_DIR}/classes")                      # для .class файлов
set(LIB_DIR "${OUTPUT_DIR}/lib")                            # для libJNIdll.so
set(HDR_DIR "${CMAKE_SOURCE_DIR}/src/simulation")           # для сгенерированного .h

file(MAKE_DIRECTORY ${CLASS_DIR} ${LIB_DIR})

if(NOT EXISTS ${JSON_JAR})
    message(FATAL_ERROR "JSON JAR not found at ${JSON_JAR}. Please download it.")
endif()

if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found via find_package. Assuming system header.")
    include_directories(/usr/include)
endif()

# 5. Экспорт переменных для подкаталогов

set(NOC_SIM_JSON_JAR ${JSON_JAR} CACHE INTERNAL "")
set(NOC_SIM_CLASS_DIR ${CLASS_DIR} CACHE INTERNAL "")
set(NOC_SIM_LIB_DIR ${LIB_DIR} CACHE INTERNAL "")
set(NOC_SIM_HDR_DIR ${HDR_DIR} CACHE INTERNAL "")
set(NOC_SIM_JNI_INCLUDES ${JNI_INCLUDE_DIRS} CACHE INTERNAL "")
set(NOC_SIM_JVM_LIBRARY ${JAVA_JVM_LIBRARY} CACHE INTERNAL "")

# 6. Подключение модулей

add_subdirectory(src)
add_subdirectory(tests)

# 7. Цель для запуска

set(JAVA_MAIN_CLASS "InterconnectGUI")

add_custom_target(run
    COMMAND ${CMAKE_COMMAND} -E echo "Starting simulation..."
    COMMAND ${Java_JAVA_EXECUTABLE}
        -Djava.library.path=${NOC_SIM_LIB_DIR}              # где искать библиотеки
        -cp "${NOC_SIM_CLASS_DIR}:${NOC_SIM_JSON_JAR}"      # classpath
        ${JAVA_MAIN_CLASS}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running InterconnectGUI..."
    DEPENDS JNIdll
    VERBATIM
)

