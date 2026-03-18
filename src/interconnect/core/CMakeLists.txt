# =========================================================
# Interconnect Core - Header-only library
# =========================================================

# создание интерфейсной библиотеки (только заголовки)
add_library(interconnect_core INTERFACE)

# настройка путей включения
target_include_directories(interconnect_core
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# установка стандарта C++ 17
target_compile_features(interconnect_core
    INTERFACE
        cxx_std_17
)

# опционально: можно указать, что это header-only для IDE
set_target_properties(interconnect_core
    PROPERTIES
        LINKER_LANGUAGE CXX
)
