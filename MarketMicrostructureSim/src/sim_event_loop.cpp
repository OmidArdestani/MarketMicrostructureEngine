#include <sim_event_loop.h>


using namespace MarketMicroStructure;

EventLoop::EventLoop(MatchingEngine& engine)
    : engine_(engine)
{}

std::thread EventLoop::runAsync(EventLoopBuffer &events)
{
    return std::thread(&EventLoop::run, this, std::ref(events));
}

void EventLoop::run(EventLoopBuffer &events)
{
    while(!WaitForDone)
    {
        while(!events.empty())
        {
            auto ev = events.pop();
            if(ev)
            {
                switch (ev->type)
                {
                case EngineEvent::Type::New:
                    engine_.handleNewOrder(ev->new_order, ev->ts_ns);
                    break;
                case EngineEvent::Type::Cancel:
                    engine_.handleCancel(ev->cancel);
                    break;
                }
            }
        }
    }
}
