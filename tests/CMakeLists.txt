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
gtest_discover_tests(port_tests)

# 2. Arbiter Tests
add_executable(arbiter_tests rr_arbiter_test.cpp)
target_link_libraries(arbiter_tests PRIVATE
    GTest::GTest
    GTest::Main
    pthread
    interconnect_core
)
target_include_directories(arbiter_tests PRIVATE ${TEST_INCLUDE_DIRS})
gtest_discover_tests(arbiter_tests)

# 4. Mesh Utils Tests
add_executable(mesh_utils_tests mesh_utils_test.cpp)
target_link_libraries(mesh_utils_tests PRIVATE
    GTest::GTest
    GTest::Main
    pthread
    interconnect_core
    mesh_lib
)
target_include_directories(mesh_utils_tests PRIVATE ${TEST_INCLUDE_DIRS})
gtest_discover_tests(mesh_utils_tests)

# 5. Mesh Router Tests
add_executable(mesh_router_tests mesh_router_test.cpp)
target_link_libraries(mesh_router_tests PRIVATE
    GTest::GTest
    GTest::Main
    pthread
    interconnect_core
    mesh_lib
)
target_include_directories(mesh_router_tests PRIVATE ${TEST_INCLUDE_DIRS})
gtest_discover_tests(mesh_router_tests)

# 6. Mesh Integration Tests
add_executable(mesh_integration_tests mesh_test.cpp)
target_link_libraries(mesh_integration_tests
    PRIVATE
    GTest::GTest
    GTest::Main
    pthread
    interconnect_core
    mesh_lib
)
target_include_directories(mesh_integration_tests PRIVATE ${TEST_INCLUDE_DIRS})
gtest_discover_tests(mesh_integration_tests)

# 7. Глобальная цель для запуска всех тестов
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --test-dir ${CMAKE_CURRENT_BINARY_DIR} --output-on-failure
    DEPENDS port_tests arbiter_tests mesh_integration_tests mesh_utils_tests mesh_router_tests
    COMMENT "Running all NoC Simulator unit tests..."
    VERBATIM
)

# Опционально: цель для запуска только mesh тестов
add_custom_target(run_mesh_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --test-dir ${CMAKE_CURRENT_BINARY_DIR} --output-on-failure -R mesh_tests
    DEPENDS mesh_tests
    COMMENT "Running Mesh topology tests..."
    VERBATIM
)
