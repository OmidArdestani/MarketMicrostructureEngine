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

#include <atomic>
#include <memory>
#include <thread>

#include "HPRingBuffer.hpp"

namespace MarketMicroStructure
{
using EventLoopBuffer = HPRingBuffer<HFTToolset::EngineEvent, 8192>;

/// @brief Creates a heap-allocated EventLoopBuffer.
/// The buffer is ~9 MB (EngineEvent is 1152 bytes x 8192 slots) and
/// must NOT be placed on the stack to avoid stack overflow.
inline std::unique_ptr<EventLoopBuffer> makeEventLoopBuffer()
{
    return std::make_unique<EventLoopBuffer>();
}

class EventLoop
{
public:
    explicit EventLoop( HFTToolset::MatchingEngine& engine );
    void run( EventLoopBuffer& events );
    std::thread runAsync( EventLoopBuffer& events );

    void setWaitForDone() { wait_for_done_.store( true, std::memory_order_release ); }
    bool isDone() const { return wait_for_done_.load( std::memory_order_acquire ); }

private:
    HFTToolset::MatchingEngine& engine_;
    std::atomic<bool> wait_for_done_{ false };
};

}  // namespace MarketMicroStructure
