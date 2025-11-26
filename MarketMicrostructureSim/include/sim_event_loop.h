#pragma once

#include <vector>
#include <types.h>
#include <matching_engine.h>

namespace MarketMicroStructure {

struct EngineEvent {
    enum class Type { New, Cancel } type;
    NewOrder    new_order{};
    CancelOrder cancel{};
    std::uint64_t ts_ns{};
};

class EventLoop {
public:
    explicit EventLoop(MatchingEngine& engine);
    void run(const std::vector<EngineEvent>& events);

private:
    MatchingEngine& engine_;
};

} // namespace ms
