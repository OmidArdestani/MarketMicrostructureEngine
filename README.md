# MarketMicrostructureEngine

A high-performance C++ matching engine and market simulation platform built on top of the HFTToolset library. This project provides a lock-free, multi-threaded simulation environment for testing trading strategies and analyzing market microstructure dynamics.

## Overview

MarketMicrostructureEngine is a sophisticated financial market simulation platform that leverages the HFTToolset library for core components including limit order book, lock-free ring buffer, and performance measurement utilities. The engine implements price-time priority matching and is designed for high throughput and low latency event processing.

## Key Features

- **Full Limit Order Book**: Price-time priority matching via HFTToolset's OrderBook implementation
- **High-Performance Event Loop**: Lock-free SPSC ring buffer (HPRingBuffer from HFTToolset) for efficient event processing
- **Multi-Symbol Support**: Handle multiple trading symbols simultaneously
- **Order Types**: Support for Limit and Market orders
- **Time-in-Force Options**: Day, IOC (Immediate-or-Cancel), and FOK (Fill-or-Kill) order types
- **Market Data Publishing**: Real-time callbacks for trades, top-of-book updates, and depth snapshots
- **O(1) Order Operations**: Constant-time order cancellation via indexed lookups
- **Thread-Safe**: Asynchronous event processing with lock-free data structures
- **Performance Measurement**: Built-in ScopeTimer from HFTToolset for latency tracking
- **Modern C++23**: Utilizes latest C++ features for optimal performance

## Architecture

The project integrates with HFTToolset as a Git submodule and builds a simulation on top of its components:

### HFTToolset Components (external/)

The HFTToolset library provides the foundational building blocks:

- **OrderBook**: Price-time priority matching engine
- **MatchingEngine**: Multi-symbol order book coordinator
- **MarketDataPublisher**: Event-driven market data callbacks
- **HPRingBuffer**: Lock-free SPSC ring buffer
- **ScopeTimer**: RAII-based performance measurement
- **Types**: Core market microstructure data structures

### Simulation Layer (src/)

Built on top of HFTToolset:

- **EventLoop**: High-performance event processing using HPRingBuffer
- **ScenarioLoader**: Load and replay trading scenarios from files
- **Main**: Simulation entry point with random event generation

## Project Structure

```
MarketMicrostructureEngine/
├── CMakeLists.txt                          # Root CMake configuration
├── LICENSE                                  # MIT License
├── README.md                                # This file
├── external/
│   └── HFTToolset/                         # HFTToolset submodule
│       ├── CMakeLists.txt
│       ├── README.md
│       ├── examples/
│       │   └── p99_example.hpp             # Latency benchmarking examples
│       └── src/
│           ├── HPRingBuffer.hpp            # Lock-free SPSC ring buffer
│           ├── ScopeTimer.hpp              # Performance measurement
│           ├── benchmark_p99.hpp           # P99/P99.9 latency tracking
│           ├── library_anchor.cpp
│           └── Market/
│               ├── order_book.h            # OrderBook interface
│               ├── order_book.cpp          # OrderBook implementation
│               ├── matching_engine.h       # MatchingEngine interface
│               ├── matching_engine.cpp     # MatchingEngine implementation
│               ├── market_data_publisher.h # Market data callbacks
│               ├── market_data_publisher.cpp
│               └── types.h                 # Market microstructure types
├── include/                                # Simulation headers
│   ├── HPRingBuffer.hpp                    # Ring buffer reference
│   ├── scenario_loader.h                   # Scenario loading
│   └── sim_event_loop.h                    # Event loop
├── src/                                    # Simulation implementation
│   ├── main.cpp                            # Simulation entry point
│   ├── scenario_loader.cpp                 # Scenario loader implementation
│   └── sim_event_loop.cpp                  # Event loop implementation
└── data/
    └── sample_scenario.txt                 # Sample trading scenario
```

## Requirements

- **CMake**: Version 3.20 or higher
- **C++ Compiler**: Support for C++23 standard
  - GCC 12+ or Clang 15+ recommended
- **Operating System**: Linux, macOS, or Windows
- **Git**: For cloning the repository and submodules

## Building

### Standard Build

```bash
# Clone the repository with submodules
git clone --recursive https://github.com/OmidArdestani/MarketMicrostructureEngine.git
cd MarketMicrostructureEngine

# If already cloned without --recursive, initialize submodules
git submodule update --init --recursive

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
- HFTToolset library components
- Simulation executable (`MarketMicrostructureSim`) to `bin/`

## Usage

### Running the Simulation

The simulation generates random orders across multiple symbols (XAUUSD, EURUSD, BTCUSD):

```bash
# From the build directory
./MarketMicrostructureSim
```

The default simulation:
- Processes 1,000,000 events
- Uses a lock-free ring buffer of 8,192 elements
- Generates random limit orders and cancellations
- Measures overall execution time with ScopeTimer

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
        .ts_ns = static_cast<std::uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count())
    };
    events.push(event);
    
    // Signal completion and wait
    loop.setWaitForDone();
    task.join();
    
    return 0;
}
```

## Core Components

### HFTToolset Integration

The project leverages the following components from HFTToolset:

#### OrderBook

The `OrderBook` class from HFTToolset maintains separate bid and ask queues with price-time priority:

- **Add Order**: `addOrder()` - Add resting limit order to the book
- **Cancel Order**: `cancelOrder()` - Remove order with O(1) lookup
- **Match Order**: `matchIncoming()` - Match aggressive order against book
- **Query Best Price**: `bestBid()`, `bestAsk()` - Get best bid/ask
- **Depth Snapshot**: `bids()`, `asks()` - Get market depth

#### MatchingEngine

The `MatchingEngine` from HFTToolset coordinates multiple order books:

- **Symbol Management**: `addSymbol()` - Register new trading symbols
- **Order Processing**: `handleNewOrder()` - Process new orders
- **Cancellation**: `handleCancel()` - Cancel existing orders
- **Multi-Symbol Support**: Manages separate order books per symbol

#### MarketDataPublisher

Event-driven market data distribution from HFTToolset:

- **Trade Callbacks**: `onTrade()` - Subscribe to trade executions
- **Top-of-Book Updates**: `onTopOfBook()` - Subscribe to best bid/ask
- **Depth Snapshots**: `onDepthSnapshot()` - Subscribe to order book depth

#### HPRingBuffer

Lock-free, single-producer single-consumer (SPSC) ring buffer from HFTToolset:

- **Thread-Safe Push**: Lock-free writes for single producer
- **Efficient Pop**: Single consumer reads without contention
- **Power-of-Two Size**: Optimized for performance with modulo operations
- **Move Semantics**: Support for efficient object transfer

#### ScopeTimer

RAII-based performance measurement from HFTToolset:

- **Automatic Timing**: Start on construction, stop on destruction
- **Compile-Time Control**: Can be disabled via preprocessor flags
- **Lightweight**: Minimal overhead for production builds

### Simulation Components

Built on top of HFTToolset:

#### EventLoop

The `EventLoop` class processes events asynchronously using HPRingBuffer:

- **Async Execution**: `runAsync()` - Process events in background thread
- **Event Processing**: Pulls events from ring buffer and dispatches to MatchingEngine
- **Graceful Shutdown**: `setWaitForDone()` - Signal completion

#### ScenarioLoader

Loads trading scenarios from configuration files:

- **File Parsing**: Read order sequences from text files
- **Event Generation**: Convert scenario data to EngineEvent objects
- **Replay Support**: Execute pre-defined trading scenarios

## Performance Characteristics

- **Order Submission**: O(log n) for price level insertion
- **Order Cancellation**: O(1) via indexed lookup
- **Order Matching**: O(1) for best price access, O(k) for k matches
- **Memory**: Efficient use of std::list for price queues
- **Event Processing**: Lock-free SPSC ring buffer for minimal latency
- **Throughput**: Capable of processing millions of events per second

## HFTToolset

This project is built on top of [HFTToolset](https://github.com/OmidArdestani/HFTToolset), a collection of high-performance C++23 utilities for low-latency applications. HFTToolset provides:

- Lock-free data structures (SPSC ring buffer)
- Performance measurement tools (p99 latency tracking, scope timers)
- Market microstructure components (order book, matching engine)
- Zero-cost abstractions with compile-time configurability

For more information about HFTToolset components, see the [HFTToolset documentation](external/HFTToolset/README.md).

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

Copyright (c) 2025 Omid Ardestani

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## Future Enhancements

Potential areas for development:

- [ ] Enhanced scenario replay from file (currently in development)
- [ ] Order book visualization
- [ ] Advanced order types (Stop, Stop-Limit, Iceberg, etc.)
- [ ] Market maker functionality
- [ ] Performance benchmarking suite with p99 latency tracking
- [ ] Network protocol support (FIX, ITCH)
- [ ] Historical data replay
- [ ] Strategy backtesting framework
- [ ] Multi-producer support for HPRingBuffer
- [ ] Order book delta updates for efficient market data

## Related Projects

- **[HFTToolset](https://github.com/OmidArdestani/HFTToolset)**: The foundational library providing core components for this project

## Author

**Omid Ardestani**

## Acknowledgments

Built with modern C++23 features for maximum performance and type safety.