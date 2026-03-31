# =========================================================
# Simulation (JNI Wrapper)
# =========================================================

# Опционально - отключаем JNI для тестирования
option(BUILD_JNI "Build JNI module" OFF)

if(BUILD_JNI)
    # 1. Определение путей
    set(JAVA_SRC_DIR "${CMAKE_SOURCE_DIR}/src/interface")
    set(JAVA_MAIN_CLASS "InterconnectGUI")
    set(JAVA_PACKAGE_PATH "${JAVA_SRC_DIR}/${JAVA_MAIN_CLASS}.java")

    # note: имя сгенерированного header будет другим, если оформим .java в package
    set(GENERATED_JNI_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/InterconnectGUI.h")

    # 2. Генерация JNI header
    add_custom_command(
        OUTPUT ${GENERATED_JNI_HEADER}
        COMMAND ${Java_JAVAC_EXECUTABLE}
            -h ${CMAKE_CURRENT_SOURCE_DIR}         # куда кладем .h (в simulation/)
            -d ${NOC_SIM_CLASS_DIR}                # куда кладем .class (в общую папку классов)
            -sourcepath ${JAVA_SRC_DIR}            # где искать исходники Java
            -cp ${NOC_SIM_JSON_JAR}                # для org.json
            ${JAVA_PACKAGE_PATH}                   # сам файл .java
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating JNI header from InterconnectGUI.java..."
        VERBATIM
    )

    add_custom_target(generate_jni_header
        DEPENDS ${GENERATED_JNI_HEADER}
    )

    # 3. Сборка Shared Library (.so)
    set(SIMULATION_SOURCES
        jni_wrapper.cpp
        InterconnectSimulator.cpp
    )

    add_library(JNIdll SHARED ${SIMULATION_SOURCES})

    # 4. Include directories
    target_include_directories(JNIdll
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${NOC_SIM_JNI_INCLUDES}
    )

    # 5. Linkage
    target_link_libraries(JNIdll
        PRIVATE
            ${NOC_SIM_JVM_LIBRARY}
            interconnect_core
    )

    # 6. Properties
    set_target_properties(JNIdll PROPERTIES
        PREFIX "lib"
        OUTPUT_NAME "JNIdll"
        LIBRARY_OUTPUT_DIRECTORY ${NOC_SIM_LIB_DIR}
    )

    # 7. Dependencies
    add_dependencies(JNIdll generate_jni_header)

    message(STATUS "Configured JNI Simulation module. Header: ${GENERATED_JNI_HEADER}")
else()
    message(STATUS "JNI module disabled (BUILD_JNI=OFF)")
endif()
