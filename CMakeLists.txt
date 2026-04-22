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
option(BUILD_JNI "Build JNI module" ON)
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
set(JNI_HDR_DIR "${CMAKE_SOURCE_DIR}/src/JNI")

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

# =========================================================
# Цель для запуска GUI
# =========================================================

if(BUILD_JNI)
    # Основная цель запуска
    add_custom_target(run
        COMMAND ${CMAKE_COMMAND} -E echo "========================================="
        COMMAND ${CMAKE_COMMAND} -E echo "Starting Interconnect Simulator GUI..."
        COMMAND ${CMAKE_COMMAND} -E echo "========================================="
        COMMAND ${CMAKE_COMMAND} -E echo "Java library path: ${NOC_SIM_LIB_DIR}"
        COMMAND ${CMAKE_COMMAND} -E echo "Classpath: ${NOC_SIM_CLASS_DIR}:${NOC_SIM_JSON_JAR}"
        COMMAND ${CMAKE_COMMAND} -E echo "========================================="
        COMMAND ${Java_JAVA_EXECUTABLE}
            -Djava.library.path=${NOC_SIM_LIB_DIR}
            -cp "${NOC_SIM_CLASS_DIR}:${NOC_SIM_JSON_JAR}"
            Interface.InterconnectApp
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running Interconnect Simulator GUI"
        DEPENDS JNIdll
        VERBATIM
    )

    # Цель для запуска с отладкой JNI
    add_custom_target(debug
        COMMAND ${CMAKE_COMMAND} -E echo "========================================="
        COMMAND ${CMAKE_COMMAND} -E echo "Starting in DEBUG mode with JNI checks..."
        COMMAND ${CMAKE_COMMAND} -E echo "========================================="
        COMMAND ${Java_JAVA_EXECUTABLE}
            -Djava.library.path=${NOC_SIM_LIB_DIR}
            -Xcheck:jni
            -cp "${NOC_SIM_CLASS_DIR}:${NOC_SIM_JSON_JAR}"
            Interface.InterconnectApp
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running GUI with JNI debugging"
        DEPENDS JNIdll
        VERBATIM
    )

    # Цель для запуска с отладочным выводом C++
    add_custom_target(run-verbose
        COMMAND ${CMAKE_COMMAND} -E echo "========================================="
        COMMAND ${CMAKE_COMMAND} -E echo "Starting with verbose C++ output..."
        COMMAND ${CMAKE_COMMAND} -E echo "========================================="
        COMMAND ${Java_JAVA_EXECUTABLE}
            -Djava.library.path=${NOC_SIM_LIB_DIR}
            -Dcpp.verbose=true
            -cp "${NOC_SIM_CLASS_DIR}:${NOC_SIM_JSON_JAR}"
            Interface.InterconnectApp
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running GUI with verbose C++ output"
        DEPENDS JNIdll
        VERBATIM
    )

    message(STATUS "")
    message(STATUS "========================================")
    message(STATUS "Available run targets:")
    message(STATUS "  make run         - Start GUI normally")
    message(STATUS "  make debug       - Start GUI with JNI checks")
    message(STATUS "  make run-verbose - Start GUI with verbose C++ output")
    message(STATUS "========================================")
endif()

# =========================================================
# Doxygen Documentation
# =========================================================

find_package(Doxygen QUIET)

if(DOXYGEN_FOUND)
    # Настройка Doxyfile из шаблона
    set(DOXYGEN_INPUT_DIR "${CMAKE_SOURCE_DIR}/src")
    set(DOXYGEN_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/docs")
    set(DOXYGEN_PROJECT_NAME "NoC Simulator JNI")
    set(DOXYGEN_PROJECT_VERSION "1.0")

    configure_file(${CMAKE_SOURCE_DIR}/Doxyfile.in ${CMAKE_BINARY_DIR}/Doxyfile @ONLY)

    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )

    message(STATUS "Doxygen found. Use 'make docs' to generate documentation.")
else()
    message(STATUS "Doxygen not found. Documentation generation disabled.")
endif()

# =========================================================
# Информация о сборке
# =========================================================

message(STATUS "")
message(STATUS "========================================")
message(STATUS "Build Configuration:")
message(STATUS "  BUILD_JNI: ${BUILD_JNI}")
message(STATUS "  BUILD_TESTS: ${BUILD_TESTS}")
message(STATUS "  Output directory: ${OUTPUT_DIR}")
message(STATUS "  JNI headers: ${JNI_HDR_DIR}")
message(STATUS "========================================")
