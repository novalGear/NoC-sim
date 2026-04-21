# =========================================================
# Network-on-Crystal Simulation Root
# =========================================================

cmake_minimum_required(VERSION 3.10)
project(NoC_Simulator_JNI CXX)

# Глобальные настройки
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Опции сборки
option(BUILD_JNI "Build JNI module" ON)  # Включаем по умолчанию
option(BUILD_TESTS "Build tests" ON)

# Поиск зависимостей
find_package(Java REQUIRED)
include(UseJava)
find_package(JNI REQUIRED)

if(BUILD_TESTS)
    find_package(GTest REQUIRED)
    include(GoogleTest)
endif()

find_package(nlohmann_json QUIET)

message(STATUS "Java found: ${Java_JAVA_EXECUTABLE}")
message(STATUS "JNI include dirs: ${JNI_INCLUDE_DIRS}")

# Пути проекта
set(JSON_JAR "${CMAKE_SOURCE_DIR}/lib/java/json-20231013.jar")

set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/output")
set(CLASS_DIR "${OUTPUT_DIR}/classes")
set(LIB_DIR "${OUTPUT_DIR}/lib")
set(JNI_HDR_DIR "${CMAKE_SOURCE_DIR}/src/JNI")  # Папка для сгенерированных хедеров

file(MAKE_DIRECTORY ${CLASS_DIR} ${LIB_DIR} ${JNI_HDR_DIR})

# Проверка JSON JAR
if(NOT EXISTS ${JSON_JAR})
    message(FATAL_ERROR "JSON JAR not found at ${JSON_JAR}")
endif()

# Проверка nlohmann_json
if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found, assuming system header")
endif()

# Экспорт переменных для подкаталогов
set(NOC_SIM_JSON_JAR ${JSON_JAR} CACHE INTERNAL "")
set(NOC_SIM_CLASS_DIR ${CLASS_DIR} CACHE INTERNAL "")
set(NOC_SIM_LIB_DIR ${LIB_DIR} CACHE INTERNAL "")
set(NOC_SIM_JNI_HDR_DIR ${JNI_HDR_DIR} CACHE INTERNAL "")
set(NOC_SIM_JNI_INCLUDES ${JNI_INCLUDE_DIRS} CACHE INTERNAL "")
set(NOC_SIM_JVM_LIBRARY ${JAVA_JVM_LIBRARY} CACHE INTERNAL "")
set(BUILD_JNI ${BUILD_JNI} CACHE INTERNAL "")

# Подключение модулей
add_subdirectory(src)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Информация о сборке
message(STATUS "========================================")
message(STATUS "Build Configuration:")
message(STATUS "  BUILD_JNI: ${BUILD_JNI}")
message(STATUS "  BUILD_TESTS: ${BUILD_TESTS}")
message(STATUS "  Output directory: ${OUTPUT_DIR}")
message(STATUS "  JNI headers will be generated in: ${JNI_HDR_DIR}")
message(STATUS "========================================")
