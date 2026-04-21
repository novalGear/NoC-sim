# =========================================================
# JNI Wrapper Module
# =========================================================

if(BUILD_JNI)
    # 1. Пути к Java исходникам
    set(JAVA_SRC_DIR "${CMAKE_SOURCE_DIR}/src/Interface")
    set(JAVA_MAIN_CLASS "service/NativeInterface")

    # Список ВСЕХ Java файлов для компиляции
    file(GLOB_RECURSE JAVA_SOURCES "${JAVA_SRC_DIR}/*.java")

    # Имя генерируемого хедера
    set(GENERATED_JNI_HEADER "${NOC_SIM_JNI_HDR_DIR}/Interface_service_NativeInterface.h")

    # 2. Компиляция ВСЕХ Java файлов и генерация JNI хедера
    add_custom_command(
        OUTPUT ${GENERATED_JNI_HEADER}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${NOC_SIM_CLASS_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${NOC_SIM_JNI_HDR_DIR}
        COMMAND ${Java_JAVAC_EXECUTABLE}
            -h ${NOC_SIM_JNI_HDR_DIR}              # Куда класть .h файлы
            -d ${NOC_SIM_CLASS_DIR}                # Куда класть .class файлы
            -sourcepath ${JAVA_SRC_DIR}            # Где искать исходники
            -cp ${NOC_SIM_JSON_JAR}                # Classpath для зависимостей
            ${JAVA_SOURCES}                        # Компилируем ВСЕ файлы
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Compiling all Java files and generating JNI header..."
        DEPENDS ${JAVA_SOURCES}
        VERBATIM
    )

    # 3. Кастомная цель для генерации хедера
    add_custom_target(generate_jni_header
        DEPENDS ${GENERATED_JNI_HEADER}
        COMMENT "Generating JNI headers"
    )

    # 4. Исходники C++
    set(JNI_SOURCES
        jni_wrapper.cpp
        # Добавь другие C++ файлы если нужно
    )

    # 5. Создание shared library
    add_library(JNIdll SHARED ${JNI_SOURCES})

    # 6. Include директории
    target_include_directories(JNIdll
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${NOC_SIM_JNI_HDR_DIR}                 # Для сгенерированного хедера
            ${NOC_SIM_JNI_INCLUDES}                # JNI системные хедеры
    )

    # 7. Линковка с существующими библиотеками
    target_link_libraries(JNIdll
        PRIVATE
            ${NOC_SIM_JVM_LIBRARY}
            mesh_lib          # Линкуем с mesh библиотекой
            network_lib       # Линкуем с network библиотекой
    )

    # 8. Свойства библиотеки
    set_target_properties(JNIdll PROPERTIES
        PREFIX "lib"
        OUTPUT_NAME "JNIdll"
        LIBRARY_OUTPUT_DIRECTORY ${NOC_SIM_LIB_DIR}
        POSITION_INDEPENDENT_CODE ON
    )

    # 9. Зависимости
    add_dependencies(JNIdll generate_jni_header)
    add_dependencies(JNIdll mesh_lib network_lib)  # Ждём сборки core библиотек

    # 10. Копирование JSON JAR в output
    add_custom_command(TARGET JNIdll POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${NOC_SIM_JSON_JAR}
            ${NOC_SIM_LIB_DIR}/
        COMMENT "Copying JSON JAR to output directory"
    )

    message(STATUS "JNI Module configured:")
    message(STATUS "  Java sources: ${JAVA_SOURCES}")
    message(STATUS "  Generated header: ${GENERATED_JNI_HEADER}")
    message(STATUS "  Output library: ${NOC_SIM_LIB_DIR}/libJNIdll.so")
else()
    message(STATUS "JNI module disabled (BUILD_JNI=OFF)")
endif()
