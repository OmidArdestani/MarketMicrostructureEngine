project(MarketMicrostructureSim LANGUAGES CXX)

add_executable(MarketMicrostructureSim)
target_sources(MarketMicrostructureSim
    PRIVATE
        src/sim_event_loop.cpp
        src/scenario_loader.cpp     # if you implement it; otherwise remove
        src/main.cpp
        include/sim_event_loop.h
        include/scenario_loader.h
)

target_include_directories(MarketMicrostructureEngineLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(MarketMicrostructureSim
    PRIVATE
        MarketMicrostructureEngineLib
)
