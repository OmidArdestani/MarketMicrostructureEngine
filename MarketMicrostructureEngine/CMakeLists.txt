project(MarketMicrostructureEngine LANGUAGES CXX)

add_library(MarketMicrostructureEngine
    src/order_book.cpp
    src/matching_engine.cpp
    src/market_data_publisher.cpp
)

target_include_directories(MarketMicrostructureEngine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
