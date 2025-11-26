cmake_minimum_required(VERSION 3.16)
project(MarketMicrostructure LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(MarketMicrostructureEngine)
# add_subdirectory(MarketMicrostructureSim)
