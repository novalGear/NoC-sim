cmake_minimum_required(VERSION 3.10)
project(NoC_Simulator_JNI CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# === 1. Поиск Java и JNI ===
find_package(JNI REQUIRED)
find_package(Java REQUIRED)

message(STATUS "Java found: ${Java_JAVA_EXECUTABLE}")
message(STATUS "JNI include dirs: ${JNI_INCLUDE_DIRS}")

# === 2. Путь к JSON библиотеке ===
set(JSON_JAR "${CMAKE_SOURCE_DIR}/lib/java/json-20231013.jar")

# Проверка существования файла (для отладки)
if(NOT EXISTS ${JSON_JAR})
    message(WARNING "JSON JAR not found at ${JSON_JAR}")
endif()

# === 3. Сборка C++ библиотеки ===
add_library(JNIdll SHARED
    src/simulation/jni_wrapper.cpp
)

target_include_directories(JNIdll PRIVATE
    ${JNI_INCLUDE_DIRS}
    src/simulation
)

set_target_properties(JNIdll PROPERTIES
    PREFIX "lib"
    OUTPUT_NAME "JNIdll"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
)

# === 4. Компиляция Java ===
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/classes)

add_custom_target(compile_java ALL
    COMMAND ${Java_JAVAC_EXECUTABLE}
        -d ${CMAKE_SOURCE_DIR}/classes
        -sourcepath ${CMAKE_SOURCE_DIR}/src/interface
        -cp ${JSON_JAR}
        ${CMAKE_SOURCE_DIR}/src/interface/InterconnectGUI.java
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Compiling Java sources..."
    VERBATIM
)

# === 5. Запуск приложения ===
# На Linux/WSL используем двоеточие как разделитель
add_custom_target(run
    COMMAND ${Java_JAVA_EXECUTABLE}
        -Djava.library.path=${CMAKE_SOURCE_DIR}/lib
        -cp ${CMAKE_SOURCE_DIR}/classes:${JSON_JAR}
        InterconnectGUI
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running InterconnectGUI..."
    DEPENDS JNIdll compile_java
    VERBATIM
)
