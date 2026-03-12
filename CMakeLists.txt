cmake_minimum_required(VERSION 3.10)
project(NoC_Simulator_JNI CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# === 1. Поиск Java и JNI ===
find_package(JNI REQUIRED)
find_package(Java REQUIRED)

include(UseJava)

message(STATUS "Java found: ${Java_JAVA_EXECUTABLE}")
message(STATUS "JNI include dirs: ${JNI_INCLUDE_DIRS}")

# === 2. Настройка путей и зависимостей ===
set(JSON_JAR "${CMAKE_SOURCE_DIR}/lib/java/json-20231013.jar")
set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/build_output")
set(CLASS_DIR "${OUTPUT_DIR}/classes")
set(LIB_DIR "${OUTPUT_DIR}/lib")
set(HDR_DIR "${CMAKE_SOURCE_DIR}/src/simulation")

file(MAKE_DIRECTORY ${CLASS_DIR})
file(MAKE_DIRECTORY ${LIB_DIR})

if(NOT EXISTS ${JSON_JAR})
    message(FATAL_ERROR "JSON JAR not found at ${JSON_JAR}. Please download it.")
endif()

find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found via find_package. Assuming system header or manual include.")
    include_directories(/usr/include)
endif()

# === 3. Генерация JNI заголовков и компиляция Java ===
# Создаем именованную цель "generate_jni_headers"
add_custom_target(generate_jni_headers ALL
    COMMAND ${Java_JAVAC_EXECUTABLE}
        -h ${HDR_DIR}
        -d ${CLASS_DIR}
        -sourcepath ${CMAKE_SOURCE_DIR}/src/interface
        -cp ${JSON_JAR}
        ${CMAKE_SOURCE_DIR}/src/interface/InterconnectGUI.java
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating JNI headers and compiling Java..."
    VERBATIM
)

# === 4. Сборка C++ библиотеки ===
add_library(JNIdll SHARED
    src/simulation/jni_wrapper.cpp
)

target_include_directories(JNIdll PRIVATE
    ${JNI_INCLUDE_DIRS}
    ${HDR_DIR}             # Путь к сгенерированному .h
    src/simulation
)

target_link_libraries(JNIdll PRIVATE ${JAVA_JVM_LIBRARY})

set_target_properties(JNIdll PROPERTIES
    PREFIX "lib"
    OUTPUT_NAME "JNIdll"
    LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR}
)

# ИСПРАВЛЕНИЕ: Зависим от имени цели, а не от пути к файлу
add_dependencies(JNIdll generate_jni_headers)

# === 5. Запуск приложения ===
# ВНИМАНИЕ: Проверьте наличие package в Java файле!
# Если в InterconnectGUI.java есть строка "package Interface;", то класс называется "Interface.InterconnectGUI"
# Если строки package нет, то просто "InterconnectGUI"
set(JAVA_MAIN_CLASS "InterconnectGUI")
# Раскомментируйте ниже, если у вас есть пакет:
# set(JAVA_MAIN_CLASS "Interface.InterconnectGUI")

add_custom_target(run
    COMMAND ${CMAKE_COMMAND} -E echo "Starting simulation..."
    COMMAND ${Java_JAVA_EXECUTABLE}
        -Djava.library.path=${LIB_DIR}
        -cp "${CLASS_DIR}:${JSON_JAR}"
        ${JAVA_MAIN_CLASS}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running InterconnectGUI..."
    DEPENDS JNIdll
    VERBATIM
)
