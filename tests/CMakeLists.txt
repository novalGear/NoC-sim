# ==============================================================================
# Модуль тестирования
# ==============================================================================

# Общие настройки включений для всех тестов
set(TEST_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/src
    ${GTEST_INCLUDE_DIRS}
)

enable_testing()

# 1. Port Tests
add_executable(port_tests port_test.cpp)
target_link_libraries(port_tests PRIVATE
    GTest::GTest
    GTest::Main
    pthread
    interconnect_core
)
target_include_directories(port_tests PRIVATE ${TEST_INCLUDE_DIRS})
gtest_discover_tests(port_tests) # Автоматическая регистрация тестов из кода

# # 2. Arbiter Tests
# add_executable(arbiter_tests rr_arbiter_test.cpp)
# target_link_libraries(arbiter_tests PRIVATE
#     GTest::GTest
#     GTest::Main
#     pthread
#     interconnect_core
# )
# target_include_directories(arbiter_tests PRIVATE ${TEST_INCLUDE_DIRS})
# gtest_discover_tests(arbiter_tests)

#
# # 3. Router Tests
# add_executable(router_tests router_test.cpp)
# target_link_libraries(router_tests PRIVATE
#     GTest::GTest
#     GTest::Main
#     pthread
#     interconnect_core
# )
# target_include_directories(router_tests PRIVATE ${TEST_INCLUDE_DIRS})
# gtest_discover_tests(router_tests)

# 4. Глобальная цель для запуска тестов
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --test-dir ${CMAKE_CURRENT_BINARY_DIR} --output-on-failure
    DEPENDS port_tests arbiter_tests router_tests
    COMMENT "Running all NoC Simulator unit tests..."
    VERBATIM
)
