// debug.hpp
#pragma once

#include <iostream>

// Включаем отладку
#define DEBUG_MESH 1

#if DEBUG_MESH
    #define DEBUG_PRINT(x) std::cout << "[DEBUG] " << x << std::endl
    #define DEBUG_PRINT_ROUTER(router, msg) \
        std::cout << "[DEBUG] Router " << router->get_id() << ": " << msg << std::endl
    #define DEBUG_PRINT_COORDS(router) \
        std::cout << "[DEBUG] Router " << router->get_id() \
                  << " at (" << router->get_coords().x << "," << router->get_coords().y << ")" << std::endl
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINT_ROUTER(router, msg)
    #define DEBUG_PRINT_COORDS(router)
#endif
