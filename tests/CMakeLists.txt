# =========================================================
# Модуль тестирования
# =========================================================

# 1. Пути включения

set(TEST_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/src
    ${GTEST_INCLUDE_DIRS}
)

# 2. Вспомогательные функции для создания тестов

function(add_noc_test TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})

    target_link_libraries(${TEST_NAME} PRIVATE
        GTest::GTest
        GTest::Main
        pthread
        interconnect_core
    )

    target_include_directories(${TEST_NAME} PRIVATE ${TEST_INCLUDE_DIRS})

    # Автоматическое обнаружение тестов
    # Эта команда сканирует исходник, находит все TEST_F и TEST,
    # и создает отдельные цели ctest для каждого.
    # TEST_PREFIX добавляет префикс имени, чтобы в отчете было понятно,
    # откуда тест (например, "port_tests.InitialState")
    gtest_discover_tests(${TEST_NAME} TEST_PREFIX "${TEST_NAME}")
endfunction()

# 3. Создание тестов

add_noc_test(port_tests port_test.cpp)
add_noc_test(arbiter_tests rr_arbiter_test.cpp)
add_noc_test(router_tests router_test.cpp)

set(ALL_TESTS port_tests arbiter_tests router_tests)

# 4. Глобальная цель для запуска тестов

add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS ${ALL_TESTS}
    COMMENT "Running all NoC Simulator unit tests..."
    VERBATIM
)

# 5. Регистрация тестов в CTest

enable_testing()
foreach(tests ${ALL_TESTS})
    add_test(NAME ${test} COMMAND ${test})
endforeach()
