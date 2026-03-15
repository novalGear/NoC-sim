# =========================================================
# Simulation (JNI Wrapper)
# =========================================================

# определение путей
set(JAVA_SRC_DIR "${CMAKE_SOURCE_SIR}/src/interface")
set(JAVA_MAIN_CLASS "InterconnectGUI")
set(JAVA_PACKAGE_PATH "${JAVA_SRC_DIR}/${JAVA_MAIN_CLASS}.java")

# note: имя сгенерированного header будет другим, если оформим .java в package
set(GENERATED_JNI_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/InterconnectGUI.hpp")

# Генерация JNI header

add_custom_command(
    OUTPUT ${GENERATED_JNI_HEADER}
    COMMAND ${Java_JAVAC_EXECUTABLE}
        -h ${CMAKE_CURRENT_DIR}         # куда кладем .h (в simulation/)
        -d ${NOC_SIM_CLASS_DIR}         # куда кладем .class (в общую папку классов)
        -sourcepath ${JAVA_SRC_DIR}     # где искать исходники Java
        -cp ${NOC_SIM_JSON_JAR}         # для org.json
        ${JAVA_PACKAGE_PATH}            # сам файл .java
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating JNI header from InterconnectGUI.java..."
    VERBATIM
)

# Сборка Shared Library (.so)

set(SIMULATION_SOURCES
    jni_wrapper.cpp
    InterconnectSimulator.cpp
)

target_include_directories(JNIdll SHARED $(SIMULATION_SOURCES))

# include directories

target_include_directories(JNIdll
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}             # InterconnectGUI.hpp и свои заголовки
        ${NOC_SIM_JNI_INCLUDES}                 # jni.h
)

# Linkage
target_link_libraries(JNIdll
    PRIVATE
        ${NOC_SIM_JVM_LIBRARY}
        interconnect_core
)

# Properties
set_target_properties(JNIdll PROPERTIES
    PREFIX "lib"                                # Linux требует префикс lib
    OUTPUT_NAME "JNIdll"                        # имя файла libJNIdll.so
    LIBRARY_OUTPUT_DIRECTORY ${NOC_SIM_LIB_DIR} # куда положить .so (build_output/lib)
)

# Dependencies
add_dependencies(JNIdll ${GENERATED_JNI_HEADER})

message(STATUS "Configured JNI Simulation module. Header: ${GENERATED_JNI_HEADER}")
