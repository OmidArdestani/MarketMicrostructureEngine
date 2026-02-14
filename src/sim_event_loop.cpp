// ============================================================================
// MarketMicrostructureEngine — EventLoop Implementation
//
// Runs in its own thread, continuously draining the lock-free ring buffer
// and routing each event to the appropriate MatchingEngine handler:
//   - NewOrder    → process_new_order()
//   - CancelOrder → process_cancel()
//
// Shutdown is signalled atomically via setWaitForDone(); the loop exits
// once the flag is raised and the buffer is observed empty.
// ============================================================================

#include <sim_event_loop.h>

using namespace MarketMicroStructure;
using namespace HFTToolset;

EventLoop::EventLoop( MatchingEngine& engine ) : engine_( engine ) {}

std::thread EventLoop::runAsync( EventLoopBuffer& events )
{
    return std::thread( &EventLoop::run, this, std::ref( events ) );
}

void EventLoop::run( EventLoopBuffer& events )
{
    while ( !isDone() )
    {
        while ( !events.empty() )
        {
            auto ev = events.pop();
            if ( ev )
            {
                switch ( ev->type )
                {
                    case EventType::NewOrder:
                        engine_.process_new_order( ev->order );
                        break;
                    case EventType::CancelOrder:
                        engine_.process_cancel( ev->cancel );
                        break;
                    default:
                        break;
                }
            }
        }
    }
}
