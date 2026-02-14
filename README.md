# MarketMicrostructureEngine

A high-performance C++23 matching engine and market microstructure simulation platform built on the HFTToolset library. This project provides a lock-free, multi-threaded simulation engine for analyzing trading dynamics and matching logic on realistic market scenarios.

## Overview

MarketMicrostructureEngine is a sophisticated financial market simulation platform that leverages the HFTToolset library for core components including limit order book matching, lock-free ring buffer (SPSC), and performance measurement utilities. The engine implements price-time priority matching and is designed for high throughput and low-latency event processing.

**Notable Achievement**: Successfully processes **1,000,000 market events** in ~700ms on a single machine.

## Key Features

- **Full Limit Order Book**: Price-time priority matching via HFTToolset's L3 Order Book
- **High-Performance Event Loop**: Lock-free SPSC ring buffer (HPRingBuffer) for zero-contention event dispatch
- **Multi-Symbol Support**: Simultaneous matching on XAUUSD, EURUSD, BTCUSD, and other symbols
- **Order Types**: Limit and Market orders with configurable time-in-force
- **Market Data Publishing**: Real-time callbacks for execution reports, trades, TOB updates, and depth snapshots
- **O(1) Order Lookups**: Constant-time order cancellation via indexed routing
- **Thread-Safe Architecture**: Atomic synchronization primitives ensure safe main-thread/worker-thread interaction
- **Performance Monitoring**: Integrated ScopeTimer for end-to-end latency tracking
- **Modern C++23**: Type-safe, move-semantic-aware event handling

## Architecture

The project integrates with HFTToolset as a Git submodule and builds a simulation on top of its components:

### HFTToolset Components (external/)

The HFTToolset library provides the foundational building blocks:

- **L3 Order Book**: Full limit order book with price-time priority matching
- **MatchingEngine**: Multi-symbol order coordinator with execution reporting
- **HPRingBuffer**: Lock-free SPSC ring buffer for zero-heap event dispatch
- **ScopeTimer**: RAII-based nanosecond-precision performance measurement
- **Clock**: High-resolution steady clock for nanosecond timestamps
- **Types**: Market microstructure data structures (Order, Trade, ExecutionReport, etc.)

### Simulation Layer (src/)

Built on top of HFTToolset for high-throughput event-driven testing:

- **EventLoop** (`sim_event_loop.h/cpp`): Asynchronous worker thread that pops events from the ring buffer and routes them to MatchingEngine handlers (process_new_order, process_cancel)
- **Main** (`main.cpp`): Simulation driver that generates random orders/cancels and pushes them to the event buffer
- **ScenarioLoader** (`scenario_loader.h/cpp`): Placeholder for future file-based scenario replay

### Threading Model

The simulation uses a **single-producer / single-consumer (SPSC)** design:

```
Main Thread (Producer)
    │
    └─→ [HPRingBuffer<EngineEvent, 8192>]  (lock-free, ~9 MB heap)
            │
            └─→ EventLoop Worker Thread (Consumer)
                    │
                    └─→ MatchingEngine
                            │
                            └─→ L3OrderBooks (one per symbol)
```

**Key Design Decisions:**
- **Heap Allocation**: The 9 MB ring buffer is heap-allocated to avoid stack overflow
- **Atomic Synchronization**: The `wait_for_done_` flag is `std::atomic<bool>` to ensure safe inter-thread signaling
- **No Lock Contention**: HPRingBuffer is lock-free, enabling millions of events per second throughput

## Project Structure

```
MarketMicrostructureEngine/
├── CMakeLists.txt                          # Root CMake build configuration
├── LICENSE                                  # MIT License
├── README.md                                # This file
├── external/
│   └── HFTToolset/                         # HFTToolset submodule (do not modify)
│       ├── CMakeLists.txt
│       ├── README.md
│       └── src/
│           ├── HPRingBuffer.hpp            # Lock-free SPSC ring buffer
│           ├── ScopeTimer.hpp              # RAII timing utility
│           ├── common/
│           │   ├── types.h                 # Core market data structures
│           │   ├── clock.h                 # High-resolution timing
│           │   └── constants.h
│           ├── market/
│           │   ├── matching_engine.h/cpp   # Multi-symbol coordinator
│           │   ├── order_book.h/cpp        # Price-time priority matching
│           │   ├── market_data_publisher.h/cpp
│           │   └── trade_engine.h/cpp
│           ├── orderbook/
│           │   ├── l3_order_book.h/cpp     # Full limit order book
│           │   ├── l2_aggregator.h/cpp
│           │   └── l1_feed.h/cpp
│           ├── risk/
│           │   └── risk_engine.h/cpp
│           ├── latency/
│           │   └── latency_model.h/cpp
│           └── metrics/
│               ├── telemetry.h/cpp
│               └── latency_histogram.h/cpp
├── include/
│   ├── sim_event_loop.h                    # Event loop interface
│   └── scenario_loader.h                   # Placeholder for scenario loading
└── src/
    ├── main.cpp                            # Simulation driver
    ├── sim_event_loop.cpp                  # Event loop implementation
    └── scenario_loader.cpp                 # Placeholder implementation
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

The default simulation generates 1,000,000 random orders and cancellations across three symbols:

```bash
# From the build directory
cd build
./MarketMicroStructureSim
```

**Expected Output:**
```
[ScopeTimer] Main Duration took 689268570ns
```

The simulation:
- Generates 1,000,000 random NewOrder and CancelOrder events
- Pushes them through a lock-free ring buffer (8,192 slots, ~9 MB heap-allocated)
- Routes each event to the MatchingEngine asynchronously via an EventLoop worker thread
- Measures end-to-end execution time with nanosecond precision

**To modify the simulation:**
- Edit `MAX_TRY` in `src/main.cpp` to change event count
- Subscribe to MatchingEngine callbacks for order fills, trades, or market data updates
- Modify `buildEvent()` to change the order generation strategy

### Integration Example

```cpp
#include <sim_event_loop.h>
#include <matching_engine.h>
#include <common/clock.h>

using namespace MarketMicroStructure;
using namespace HFTToolset;

int main() {
    Clock clock;
    MatchingEngine engine(clock);
    engine.add_symbol("BTCUSD");

    // Create event loop with heap-allocated ring buffer
    EventLoop loop(engine);
    auto events = makeEventLoopBuffer();

    // Run asynchronously in worker thread
    auto task = loop.runAsync(*events);

    // Push a new order event
    EngineEvent order_event;
    order_event.type = EventType::NewOrder;
    order_event.order = Order{
        .id = 1,
        .trader_id = 100,
        .symbol = Symbol("BTCUSD"),
        .side = Side::Buy,
        .type = OrderType::Limit,
        .tif = TimeInForce::Day,
        .price = 50000,
        .quantity = 1
    };
    order_event.event_time = clock.now();
    events->push(order_event);

    // Push a cancel event
    EngineEvent cancel_event;
    cancel_event.type = EventType::CancelOrder;
    cancel_event.cancel.order_id = 1;
    cancel_event.event_time = clock.now();
    events->push(cancel_event);

    // Drain buffer and signal completion
    while (!events->empty()) { }
    loop.setWaitForDone();
    task.join();

    return 0;
}
```


## Core Components

### HFTToolset Components (Used, Not Modified)

The project leverages these read-only HFTToolset components:

#### L3 Order Book (`orderbook/l3_order_book.h`)

Full limit order book with price-time priority matching:

- **Order Submission**: `add_order()` - Add resting limit order to the book
- **Order Cancellation**: `cancel_order()` - Remove order with O(1) indexed lookup
- **Matching**: `match_incoming()` - Aggressive order matches against resting orders
- **Best Prices**: `best_bid()`, `best_ask()` - O(1) access to best levels
- **Depth Query**: `get_bids()`, `get_asks()` - Market depth snapshots
- **Callbacks**: Trade and TOB update callbacks

#### MatchingEngine (`market/matching_engine.h`)

Coordinates multiple symbol-specific order books:

- **Symbol Registration**: `add_symbol(SymbolId)` - Register new trading symbols
- **Order Routing**: `process_new_order(Order)` - Route to correct symbol book
- **Cancellation**: `process_cancel(CancelRequest)` - O(1) order-to-symbol lookup
- **Execution Reports**: Generate execution reports on fills
- **Multi-Symbol**: Maintains separate L3 books per symbol
- **Market Data**: Wires up L1 (TOB) and L2 (depth) feeds

#### HPRingBuffer (`HPRingBuffer.hpp`)

Lock-free SPSC ring buffer for zero-allocation event dispatch:

- **Lock-Free Push/Pop**: No mutexes; power-of-2 size for bitmask wrapping
- **Move Semantics**: Efficient ownership transfer with `std::move`
- **Memory-Efficient**: ~1.1 MB per 1000 slots (adjustable size)
- **Throughput**: Millions of events per second with minimal latency variance

#### ScopeTimer (`ScopeTimer.hpp`)

RAII-based nanosecond-precision performance measurement:

- **Automatic Timing**: Starts on construction, ends on destruction
- **Low Overhead**: Simple clock.now() calls in release builds
- **Thread-Local Storage**: No global state or locks
- **Named Timers**: `ScopeTimerManagement` for named timer tracking

#### Clock (`common/clock.h`)

High-resolution steady clock wrapper for `std::chrono::steady_clock`:

- **Nanosecond Precision**: Returns `Timestamp` (uint64_t nanoseconds)
- **Steady Guarantees**: Monotonic, not affected by system time adjustments

### Simulation Components (Our Implementation)

#### EventLoop (`include/sim_event_loop.h`, `src/sim_event_loop.cpp`)

Asynchronous event dispatcher running in a worker thread:

```cpp
// Usage pattern:
EventLoop loop(engine);
auto events = makeEventLoopBuffer();
auto task = loop.runAsync(*events);  // Runs in background thread

// Main thread pushes events
events->push(event1);
events->push(event2);

// Signal completion and wait
loop.setWaitForDone();
task.join();
```

**Key Design:**
- `wait_for_done_` is `std::atomic<bool>` to prevent data races
- Loop spins on ring buffer; exit condition is `isDone() && buffer.empty()`
- Routes events via switch on `EventType::` enum

**Critical Fix Applied:**
- Changed `WaitForDone` bool → `std::atomic<bool> wait_for_done_`
- Changed `setWaitForDone()` to use `memory_order_release`
- Changed `run()` to use `isDone()` with `memory_order_acquire`

#### Main Simulation (`src/main.cpp`)

Entry point that drives high-throughput event generation:

```cpp
// Heap-allocated buffer (~9 MB) to avoid stack overflow
auto events = makeEventLoopBuffer();

// Generate 1,000,000 random orders/cancels
while (MAX_TRY > 0) {
    if (events->push(buildEvent())) {
        MAX_TRY--;
    }
}
```

**Key Design:**
- `buildEvent()` generates random Order or CancelRequest
- Uses `std::mt19937` RNG seeded from `std::random_device`
- Pools three trading symbols with random parameters

**Critical Fix Applied:**
- Changed `EventLoopBuffer events;` (stack) → `auto events = makeEventLoopBuffer();` (heap)
- Prevents 9 MB stack overflow on systems with 8 MB default stack size

#### ScenarioLoader (`include/scenario_loader.h`, `src/scenario_loader.cpp`)

Placeholder for future file-based scenario replay functionality:

```cpp
// TODO: Load trading scenarios from JSON/text files
// TODO: Support event sequencing with configurable delays
// TODO: Enable deterministic strategy backtesting
```

## Performance Characteristics

- **Order Submission**: O(log n) insertion to price level (binary search on first level)
- **Order Cancellation**: O(1) via order-to-symbol index in MatchingEngine
- **Order Matching**: O(1) for best price access, O(k) for k fills
- **Memory Layout**: Cache-aligned structures (64 bytes, `alignas(64)`) for efficient CPU cache usage
- **Event Throughput**: ~1.4M events/sec (1M events in ~700ms on typical hardware)
- **Lock-Free Design**: HPRingBuffer requires no mutexes; only atomics for indices

## Common Issues & Solutions

### Stack Overflow with EventLoopBuffer

**Problem**: Allocating `EventLoopBuffer events;` on the stack causes segfault.

**Root Cause**: `HPRingBuffer<EngineEvent, 8192>` is ~9 MB, exceeding typical 8 MB stack limits.

**Solution**: Use heap allocation via `auto events = makeEventLoopBuffer();`

### Data Race on WaitForDone Flag

**Problem**: Plain `bool WaitForDone;` causes undefined behavior in multi-threaded context.

**Root Cause**: Race between main thread write and event loop thread read without synchronization.

**Solution**: Use `std::atomic<bool> wait_for_done_;` with explicit memory ordering.

## Building on Different Platforms

### Linux (GCC/Clang)
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j $(nproc)
./MarketMicroStructureSim
```

### macOS
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ ..
cmake --build . -j $(sysctl -n hw.ncpu)
./MarketMicroStructureSim
```

### Windows (MSVC)
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 16 2019" ..
cmake --build . --config Release -j %NUMBER_OF_PROCESSORS%
.\Release\MarketMicroStructureSim.exe
```

## HFTToolset Integration

This project is built entirely on [HFTToolset](https://github.com/OmidArdestani/HFTToolset), a high-performance C++23 library:

- **Lock-Free Data Structures**: SPSC ring buffer with bitmask wrapping
- **Performance Tools**: Nanosecond-precision timing with zero allocations
- **Market Components**: L3 order book, matching engine, execution routing
- **Zero-Cost Abstractions**: Compile-time configurability with no runtime overhead

See [external/HFTToolset/README.md](external/HFTToolset/README.md) for component details.

## License

Licensed under the MIT License. See [LICENSE](LICENSE) for details.

Copyright © 2025 Omid Ardestani

## Contributing

Contributions, issues, and feature requests are welcome!

## Future Enhancements

- [ ] File-based scenario replay (ScenarioLoader implementation)
- [ ] Web-based order book visualization
- [ ] Advanced order types (Stop, Stop-Limit, Iceberg orders)
- [ ] Market maker simulation with inventory tracking
- [ ] P99/P99.9 latency benchmarking suite
- [ ] FIX protocol gateway
- [ ] Historical market data replay from files
- [ ] Strategy backtesting framework
- [ ] Multi-producer support for HPRingBuffer
- [ ] Order book delta compression for efficient market data distribution

## Related Projects

- **[HFTToolset](https://github.com/OmidArdestani/HFTToolset)** — Foundational library

## Author

**Omid Ardestani** — Low-latency financial systems architect

## Acknowledgments

Built with C++23 for maximum performance and type safety.
