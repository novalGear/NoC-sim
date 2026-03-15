# =========================================================
# Interconnect Core
# =========================================================

# список исходных файлов
set(INTERCONNECT_SOURCES
    router.cpp
)

set(INTERCONNECT_HEADERS
    packet.hpp
    port.hpp
    rr_arbiter.hpp
    interconnect.hpp
)

# создание библиотеки
add_library(interconnect_core STATIC ${INTERCONNECT_SOURCES})

# настройка путей включения
target_include_directories(interconnect_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# установка стандарта C++ 17
target_compile_features(interconnect_core
    PUBLIC
        cxx_std_17
)
