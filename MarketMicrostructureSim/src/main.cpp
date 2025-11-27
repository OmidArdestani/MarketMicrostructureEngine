#include <iostream>
#include <random>

#include <matching_engine.h>
#include <market_data_publisher.h>
#include <types.h>

#include <sim_event_loop.h>

using namespace MarketMicroStructure;

const std::array<SymbolId, 3> Symbols = { "XAUUSD", "EURUSD", "BTCUSD" };

EngineEvent buildEvent()
{
    static std::mt19937 rng(std::random_device{}());

    std::uniform_int_distribution<int> symbol_dist(0, Symbols.size()-1);
    std::uniform_int_distribution<int> type_dist(0, 1);
    std::uniform_int_distribution<int> price_dist(90, 110);
    std::uniform_int_distribution<int> qty_dist(1, 500);
    std::uniform_int_distribution<OrderId> order_id_dist(1, 10'000);

    int type = type_dist(rng);

    if (type == 0)
    {
        // New order event
        return EngineEvent{
            EngineEvent::Type::New,
            NewOrder{
                order_id_dist(rng),
                order_id_dist(rng),
                Symbols[symbol_dist(rng)],
                (rng() % 2 == 0 ? Side::Buy : Side::Sell),
                OrderType::Limit,
                TimeInForce::Day,
                price_dist(rng),
                qty_dist(rng)
            },
            {},
            static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count())
        };
    }
    else
    {
        // Cancel event
        return EngineEvent{
            EngineEvent::Type::Cancel,
            {},
            CancelOrder{order_id_dist(rng)},
            static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count())
        };
    }
}


int main() {
    MarketDataPublisher md_pub;

    // Subscribe to market data streams
    md_pub.onTopOfBook([](const TopOfBook& tob) {
        // std::cout << "[TOB] " << tob.symbol
        //           << " | Bid: " << tob.best_bid.price << " x " << tob.best_bid.qty
        //           << " | Ask: " << tob.best_ask.price << " x " << tob.best_ask.qty
        //           << "\n";
    });

    md_pub.onTrade([](const Trade& t) {
        // std::cout << "[TRADE] " << t.symbol
        //           << " | Px: " << t.price
        //           << " | Qty: " << t.qty
        //           << " | Aggressor: " << (t.aggressor_side == Side::Buy ? "B" : "S")
        //           << "\n";
    });

    MatchingEngine engine(md_pub);
    engine.addSymbol("XAUUSD");
    engine.addSymbol("EURUSD");
    engine.addSymbol("BTCUSD");

    EventLoop loop(engine);
    EventLoopBuffer events;
    auto task = loop.runAsync(events);

    auto start_time = std::chrono::steady_clock::now();

    uint64_t MAX_TRY{ 1000'000 };
    while( MAX_TRY > 0 )
    {
        if(events.push(buildEvent()))
        {
            MAX_TRY--;
            continue;
        }
    }

    while(!events.empty());

    loop.setWaitForDone();

    task.join();

    auto end_time = std::chrono::steady_clock::now();

    std::cout.imbue(std::locale("en_US.UTF-8"));
    std::cout << std::endl << "Job duration for 1e6 events: " << std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count() << " [us]" << std::endl;

    return 0;
}
