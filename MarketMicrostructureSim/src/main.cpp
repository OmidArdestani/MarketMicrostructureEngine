#include <iostream>
#include <vector>

#include <matching_engine.h>
#include <market_data_publisher.h>
#include <types.h>

#include <sim_event_loop.h>

using namespace MarketMicroStructure;

int main() {
    MarketDataPublisher md_pub;

    // Subscribe to market data streams
    md_pub.onTopOfBook([](const TopOfBook& tob) {
        std::cout << "[TOB] " << tob.symbol
                  << " | Bid: " << tob.best_bid.price << " x " << tob.best_bid.qty
                  << " | Ask: " << tob.best_ask.price << " x " << tob.best_ask.qty
                  << "\n";
    });

    md_pub.onTrade([](const Trade& t) {
        std::cout << "[TRADE] " << t.symbol
                  << " | Px: " << t.price
                  << " | Qty: " << t.qty
                  << " | Aggressor: " << (t.aggressor_side == Side::Buy ? "B" : "S")
                  << "\n";
    });

    MatchingEngine engine(md_pub);
    engine.addSymbol("FOO");

    EventLoop loop(engine);
    std::vector<EngineEvent> events;

    // Simple scripted scenario
    events.push_back(EngineEvent{
        EngineEvent::Type::New,
        NewOrder{1, 1001, "FOO", Side::Sell, OrderType::Limit, TimeInForce::Day, 101, 50},
        {},
        1'000'000
    });

    events.push_back(EngineEvent{
        EngineEvent::Type::New,
        NewOrder{2, 1002, "FOO", Side::Sell, OrderType::Limit, TimeInForce::Day, 102, 75},
        {},
        2'000'000
    });

    events.push_back(EngineEvent{
        EngineEvent::Type::New,
        NewOrder{3, 2001, "FOO", Side::Buy, OrderType::Limit, TimeInForce::Day, 99, 40},
        {},
        3'000'000
    });

    events.push_back(EngineEvent{
        EngineEvent::Type::New,
        NewOrder{4, 2002, "FOO", Side::Buy, OrderType::Limit, TimeInForce::Day, 102, 60},
        {},
        4'000'000
    });

    events.push_back(EngineEvent{
        EngineEvent::Type::New,
        NewOrder{5, 1003, "FOO", Side::Sell, OrderType::Market, TimeInForce::IOC, 0, 30},
        {},
        5'000'000
    });

    loop.run(events);

    return 0;
}
