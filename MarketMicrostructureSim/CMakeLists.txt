project(MarketMicrostructureSim LANGUAGES CXX)

add_library(MarketMicrostructureSimLib
    src/event_loop.cpp
    src/scenario_loader.cpp     # if you implement it; otherwise remove
)

target_include_directories(MarketMicrostructureSimLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(MarketMicrostructureSimLib
    PUBLIC
        MarketMicrostructureEngine
)

add_executable(MarketMicrostructureSim
    src/main.cpp
)

target_link_libraries(MarketMicrostructureSim
    PRIVATE
        MarketMicrostructureSimLib
)
