// ============================================================================
// MarketMicrostructureEngine — Simulation Entry Point
//
// Drives a high-throughput market simulation by generating random NewOrder
// and CancelOrder events, pushing them through a lock-free SPSC ring buffer
// (HPRingBuffer) to an asynchronous EventLoop that dispatches them into
// the HFTToolset MatchingEngine.
//
// Simulation parameters:
//   - Symbols:   XAUUSD, EURUSD, BTCUSD
//   - Events:    1,000,000 (configurable via MAX_TRY)
//   - Buffer:    8,192 slots, heap-allocated (~9 MB)
//   - Timing:    Measured end-to-end via HFTToolset ScopeTimer
// ============================================================================

#include <common/types.h>
#include <market/market_data_publisher.h>
#include <market/matching_engine.h>
#include <sim_event_loop.h>

#include <chrono>
#include <random>
#include <ScopeTimer.hpp>

using namespace MarketMicroStructure;
using namespace HFTToolset;

const std::array<Symbol, 3> Symbols = { Symbol( "XAUUSD" ), Symbol( "EURUSD" ), Symbol( "BTCUSD" ) };

EngineEvent buildEvent()
{
    static std::mt19937 rng( std::random_device{}() );

    std::uniform_int_distribution<int> symbol_dist( 0, Symbols.size() - 1 );
    std::uniform_int_distribution<int> type_dist( 0, 1 );
    std::uniform_int_distribution<int> price_dist( 90, 110 );
    std::uniform_int_distribution<int> qty_dist( 1, 500 );
    std::uniform_int_distribution<OrderId> order_id_dist( 1, 10'000 );

    int type = type_dist( rng );

    if ( type == 0 )
    {
        auto the_order = Order{ .id             = order_id_dist( rng ),
                                .trader_id      = order_id_dist( rng ),
                                .symbol         = Symbols[symbol_dist( rng )],
                                .side           = ( rng() % 2 == 0 ? Side::Buy : Side::Sell ),
                                .type           = OrderType::Limit,
                                .tif            = TimeInForce::Day,
                                .price          = price_dist( rng ),
                                .quantity       = qty_dist( rng ),
                                .filled_qty     = qty_dist( rng ),
                                .status         = OrderStatus::New,
                                .submit_time    = 0,
                                .accept_time    = 0,
                                .queue_position = 0,
                                .cl_ord_id      = {} };
        Timestamp ts   = std::chrono::steady_clock::now().time_since_epoch().count();

        // New order event
        EngineEvent ev;
        ev.type       = EventType::NewOrder;
        ev.order      = the_order;
        ev.event_time = ts;
        return ev;
    }
    else
    {
        Timestamp ts = std::chrono::steady_clock::now().time_since_epoch().count();

        // Cancel event
        EngineEvent ev;
        ev.type            = EventType::CancelOrder;
        ev.cancel.order_id = order_id_dist( rng );
        ev.event_time      = ts;
        return ev;
    }
}

int main()
{
    HFTToolset::Clock clock;

    // Subscribe to market data streams
    // md_pub.onTopOfBook([](const TopOfBook& tob) {
    //     // std::cout << "[TOB] " << tob.symbol
    //     //           << " | Bid: " << tob.best_bid.price << " x " << tob.best_bid.qty
    //     //           << " | Ask: " << tob.best_ask.price << " x " << tob.best_ask.qty
    //     //           << "\n";
    // });

    // md_pub.onTrade([](const Trade& t) {
    //     // std::cout << "[TRADE] " << t.symbol
    //     //           << " | Px: " << t.price
    //     //           << " | Qty: " << t.qty
    //     //           << " | Aggressor: " << (t.aggressor_side == Side::Buy ? "B" : "S")
    //     //           << "\n";
    // });

    HFTToolset::MatchingEngine engine( clock );
    engine.add_symbol( "XAUUSD" );
    engine.add_symbol( "EURUSD" );
    engine.add_symbol( "BTCUSD" );

    EventLoop loop( engine );

    // Heap-allocate: EventLoopBuffer is ~9 MB (EngineEvent x 8192 slots)
    // and must not live on the stack to avoid stack overflow.
    auto events = makeEventLoopBuffer();
    auto task   = loop.runAsync( *events );

    NScopeTimers::start( "Main Duration" );

    uint64_t MAX_TRY{ 1'000'000 };
    // uint64_t MAX_TRY{ 5 };
    while ( MAX_TRY > 0 )
    {
        if ( events->push( buildEvent() ) )
        {
            MAX_TRY--;
            continue;
        }
    }

    while ( !events->empty() )
        ;

    loop.setWaitForDone();

    task.join();

    NScopeTimers::endAndLog( "Main Duration" );

    return 0;
}
