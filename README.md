# MarketMicrostructureEngine

A high-performance C++ matching engine and order book implementation for simulating market microstructure. This project provides a lock-free, multi-threaded simulation environment for testing trading strategies and analyzing market dynamics.

## Overview

MarketMicrostructureEngine is a sophisticated financial market simulation platform that implements a full limit order book with price-time priority matching. The engine is designed for high throughput and low latency, featuring a lock-free ring buffer for event processing and efficient data structures for order management.

## Key Features

- **Full Limit Order Book**: Price-time priority matching with support for multiple price levels
- **High-Performance Event Loop**: Lock-free ring buffer (HPRingBuffer) for efficient event processing
- **Multi-Symbol Support**: Handle multiple trading symbols simultaneously
- **Order Types**: Support for Limit and Market orders
- **Time-in-Force Options**: Day, IOC (Immediate-or-Cancel), and FOK (Fill-or-Kill) order types
- **Market Data Publishing**: Real-time callbacks for trades, top-of-book updates, and depth snapshots
- **O(1) Order Operations**: Constant-time order cancellation via indexed lookups
- **Thread-Safe**: Asynchronous event processing with lock-free data structures
- **Modern C++23**: Utilizes latest C++ features for optimal performance

## Architecture

The project is organized into two main components:

### 1. MarketMicrostructureEngine (Core Library)

The core library provides the fundamental matching engine components:

- **OrderBook**: Manages bid/ask queues with price-time priority
- **MatchingEngine**: Coordinates multiple order books and processes orders
- **MarketDataPublisher**: Event-driven system for market data callbacks
- **Types**: Core data structures (Order, Trade, BookLevel, etc.)
- **ScopeTimer**: Performance measurement utility

### 2. MarketMicrostructureSim (Simulation)

The simulation component provides:

- **EventLoop**: High-performance event processing loop
- **HPRingBuffer**: Lock-free ring buffer for event queuing
- **ScenarioLoader**: Load and replay trading scenarios
- **Main**: Simulation entry point with configurable scenarios

## Project Structure

```
MarketMicrostructureEngine/
├── CMakeLists.txt                          # Root CMake configuration
├── LICENSE                                  # MIT License
├── README.md                                # This file
├── MarketMicrostructureEngine/             # Core engine library
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── matching_engine.h               # Main matching engine
│   │   ├── order_book.h                    # Order book implementation
│   │   ├── market_data_publisher.h         # Market data callbacks
│   │   ├── types.h                         # Core data types
│   │   └── ScopeTimer.hpp                  # Performance timer
│   └── src/
│       ├── matching_engine.cpp
│       ├── order_book.cpp
│       └── market_data_publisher.cpp
└── MarketMicrostructureSim/                # Simulation application
    ├── CMakeLists.txt
    ├── include/
    │   ├── sim_event_loop.h                # Event processing loop
    │   ├── HPRingBuffer.hpp                # Lock-free ring buffer
    │   └── scenario_loader.h               # Scenario loading
    ├── src/
    │   ├── main.cpp                        # Simulation entry point
    │   ├── sim_event_loop.cpp
    │   └── scenario_loader.cpp
    └── data/
        └── sample_scenario.txt             # Sample trading scenario
```

## Requirements

- **CMake**: Version 3.16 or higher
- **C++ Compiler**: Support for C++23 standard
  - GCC 12+ or Clang 15+ recommended
- **Operating System**: Linux, macOS, or Windows

## Building

### Standard Build

```bash
# Clone the repository
git clone https://github.com/OmidArdestani/MarketMicrostructureEngine.git
cd MarketMicrostructureEngine

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
cmake --build .
```

### Installation

```bash
# Install library and headers (optional)
cmake --build . --target install
```

This will install:
- Shared library (`libMarketMicrostructureEngineLib.so`/`.dll`) to `lib/`
- Header files to `include/`
- Executable (`MarketMicrostructureSim`) to `bin/`

## Usage

### Running the Simulation

The simulation generates random orders across multiple symbols (XAUUSD, EURUSD, BTCUSD):

```bash
# From the build directory
./MarketMicrostructureSim/MarketMicrostructureSim
```

The default simulation:
- Processes 1,000,000 events
- Uses a ring buffer of 8,192 elements
- Generates random limit orders and cancellations
- Measures overall execution time

### Basic Integration Example

```cpp
#include <matching_engine.h>
#include <market_data_publisher.h>
#include <types.h>

using namespace MarketMicroStructure;

int main() {
    // Create market data publisher
    MarketDataPublisher md_pub;
    
    // Subscribe to market data
    md_pub.onTrade([](const Trade& t) {
        std::cout << "Trade: " << t.symbol 
                  << " @ " << t.price 
                  << " x " << t.qty << "\n";
    });
    
    md_pub.onTopOfBook([](const TopOfBook& tob) {
        std::cout << "TOB: " << tob.symbol
                  << " Bid: " << tob.best_bid.price
                  << " Ask: " << tob.best_ask.price << "\n";
    });
    
    // Create matching engine
    MatchingEngine engine(md_pub);
    engine.addSymbol("AAPL");
    
    // Submit a new order
    NewOrder order{
        .id = 1,
        .trader = 100,
        .symbol = "AAPL",
        .side = Side::Buy,
        .type = OrderType::Limit,
        .tif = TimeInForce::Day,
        .price = 150,
        .qty = 100
    };
    
    engine.handleNewOrder(order, std::chrono::steady_clock::now().time_since_epoch().count());
    
    return 0;
}
```

### Advanced Usage with Event Loop

```cpp
#include <sim_event_loop.h>
#include <matching_engine.h>

using namespace MarketMicroStructure;

int main() {
    MarketDataPublisher md_pub;
    MatchingEngine engine(md_pub);
    engine.addSymbol("BTCUSD");
    
    // Create event loop with ring buffer
    EventLoop loop(engine);
    EventLoopBuffer events;
    
    // Run asynchronously
    auto task = loop.runAsync(events);
    
    // Push events to the buffer
    EngineEvent event{
        .type = EngineEvent::Type::New,
        .new_order = { /* order details */ },
        .ts_ns = get_current_time_ns()
    };
    events.push(event);
    
    // Signal completion and wait
    loop.setWaitForDone();
    task.join();
    
    return 0;
}
```

## Core Components

### OrderBook

The `OrderBook` class maintains separate bid and ask queues with price-time priority:

- **Add Order**: `addOrder()` - Add resting limit order to the book
- **Cancel Order**: `cancelOrder()` - Remove order with O(1) lookup
- **Match Order**: `matchIncoming()` - Match aggressive order against book
- **Query Best Price**: `bestBid()`, `bestAsk()` - Get best bid/ask
- **Depth Snapshot**: `bids()`, `asks()` - Get market depth

### MatchingEngine

The `MatchingEngine` coordinates multiple order books:

- **Symbol Management**: `addSymbol()` - Register new trading symbols
- **Order Processing**: `handleNewOrder()` - Process new orders
- **Cancellation**: `handleCancel()` - Cancel existing orders
- **Multi-Symbol Support**: Manages separate order books per symbol

### MarketDataPublisher

Event-driven market data distribution:

- **Trade Callbacks**: `onTrade()` - Subscribe to trade executions
- **Top-of-Book Updates**: `onTopOfBook()` - Subscribe to best bid/ask
- **Depth Snapshots**: `onDepthSnapshot()` - Subscribe to order book depth

### HPRingBuffer

Lock-free, multi-producer single-consumer ring buffer:

- **Thread-Safe Push**: Lock-free writes for multiple producers
- **Efficient Pop**: Single consumer reads without contention
- **Power-of-Two Size**: Optimized for performance
- **Move Semantics**: Support for efficient object transfer

## Performance Characteristics

- **Order Submission**: O(log n) for price level insertion
- **Order Cancellation**: O(1) via indexed lookup
- **Order Matching**: O(1) for best price access, O(k) for k matches
- **Memory**: Efficient use of std::list for price queues
- **Throughput**: Capable of processing millions of events per second

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

Copyright (c) 2025 Omid Ardestani

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## Future Enhancements

Potential areas for development:

- [ ] Scenario replay from file
- [ ] Order book visualization
- [ ] Advanced order types (Stop, Stop-Limit, etc.)
- [ ] Market maker functionality
- [ ] Performance benchmarking suite
- [ ] Network protocol support (FIX, ITCH)
- [ ] Historical data replay
- [ ] Strategy backtesting framework

## Author

**Omid Ardestani**

## Acknowledgments

Built with modern C++23 features for maximum performance and type safety.