#include <sim_event_loop.h>

using namespace MarketMicroStructure;

EventLoop::EventLoop(MatchingEngine& engine)
    : engine_(engine)
{}

void EventLoop::run(const std::vector<EngineEvent>& events) {
    for (const auto& ev : events) {
        switch (ev.type) {
            case EngineEvent::Type::New:
                engine_.handleNewOrder(ev.new_order, ev.ts_ns);
                break;
            case EngineEvent::Type::Cancel:
                engine_.handleCancel(ev.cancel);
                break;
        }
    }
}
