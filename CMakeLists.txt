cmake_minimum_required(VERSION 3.20)
project(MarketMicroStructureSim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags for optimization and warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3 -march=native")


add_subdirectory(external/HFTToolset)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(MarketMicroStructureSim src/main.cpp)


target_sources(MarketMicroStructureSim
    PRIVATE
        src/sim_event_loop.cpp
        src/scenario_loader.cpp
        include/sim_event_loop.h
        include/scenario_loader.h
)

target_link_libraries(MarketMicroStructureSim
    PRIVATE
    HFT::Toolset
)