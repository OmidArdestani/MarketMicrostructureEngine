project(MarketMicrostructureEngine LANGUAGES CXX)

add_library(MarketMicrostructureEngine SHARED)
target_sources(MarketMicrostructureEngine
    PRIVATE
        src/order_book.cpp
        src/matching_engine.cpp
        src/market_data_publisher.cpp

    PUBLIC
        include/market_data_publisher.h
        include/matching_engine.h
        include/order_book.h
        include/types.h
)

target_include_directories(MarketMicrostructureEngine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Install the library (DLL / SO)
install(TARGETS MarketMicrostructureEngine
    RUNTIME DESTINATION bin        # Windows .dll
    LIBRARY DESTINATION lib        # Linux .so, Mac .dylib
    ARCHIVE DESTINATION lib        # Windows .lib
)

# Install headers to include/
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
)
