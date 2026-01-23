#pragma once

#include "HPRingBuffer.hpp"
#include <thread>
#include <types.h>
#include <matching_engine.h>

namespace MarketMicroStructure {

struct EngineEvent {
    enum class Type { New, Cancel } type;
    NewOrder    new_order{};
    CancelOrder cancel{};
    std::uint64_t ts_ns{};
};

using EventLoopBuffer = HPRingBuffer<EngineEvent, 8192>;

class EventLoop {
public:
    explicit EventLoop(MatchingEngine& engine);
    void run(EventLoopBuffer &events);
    std::thread runAsync(EventLoopBuffer &events);
    void setWaitForDone(){ WaitForDone = true; }

private:
    MatchingEngine& engine_;
    bool WaitForDone { false };
};

} // namespace ms
