#pragma once
// ============================================================================
// MarketMicrostructureEngine — Asynchronous Event Loop
//
// Provides a single-consumer event loop that pulls EngineEvent objects from
// a lock-free HPRingBuffer and dispatches them to the HFTToolset
// MatchingEngine.  Designed for a single-producer / single-consumer (SPSC)
// threading model:
//   - Producer (main thread) pushes events via EventLoopBuffer::push()
//   - Consumer (EventLoop thread) pops and processes via run()
//
// The EventLoopBuffer (~9 MB) MUST be heap-allocated; a convenience factory
// function makeEventLoopBuffer() is provided for this purpose.
// ============================================================================

#include <common/types.h>
#include <market/matching_engine.h>

#include <thread>

#include "HPRingBuffer.hpp"

namespace MarketMicroStructure
{
using EventLoopBuffer = HPRingBuffer<HFTToolset::EngineEvent, 8192>;

class EventLoop
{
public:
    explicit EventLoop( HFTToolset::MatchingEngine& engine );
    void run( EventLoopBuffer& events );
    std::thread runAsync( EventLoopBuffer& events );

    void setWaitForDone() { WaitForDone = true; }

private:
    HFTToolset::MatchingEngine& engine_;
    bool WaitForDone{ false };
};

}  // namespace MarketMicroStructure
