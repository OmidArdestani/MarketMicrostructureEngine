#pragma once


#include <cstdint>
#include <string>

namespace MarketMicroStructure {

using OrderId   = std::uint64_t;
using TraderId  = std::uint64_t;
using SymbolId  = std::string;
using Price     = std::int64_t;   // e.g. price in ticks
using Quantity  = std::int64_t;   // quantity in units

enum class Side : std::uint8_t {
    Buy,
    Sell
};

enum class OrderType : std::uint8_t {
    Limit,
    Market
};

enum class TimeInForce : std::uint8_t {
    Day,
    IOC,
    FOK
};

struct NewOrder {
    OrderId     id;
    TraderId    trader;
    SymbolId    symbol;
    Side        side;
    OrderType   type;
    TimeInForce tif;
    Price       price;      // ignored for market orders
    Quantity    qty;
};

struct CancelOrder {
    OrderId id;
};

struct Trade {
    OrderId     resting_id;
    OrderId     incoming_id;
    SymbolId    symbol;
    Side        aggressor_side;
    Price       price;
    Quantity    qty;
    std::uint64_t match_timestamp_ns;
};

struct BookLevel {
    Price    price;
    Quantity qty;
};

struct TopOfBook {
    SymbolId symbol;
    BookLevel best_bid;
    BookLevel best_ask;
    bool     valid{false};
};

struct BookOrder {
    OrderId   id;
    TraderId  trader;
    Quantity  qty;
    Price     price;
    Side      side;
    std::uint64_t ts_ns; // arrival time, for time priority

    BookOrder(const NewOrder& o, std::uint64_t ts_ns)
    {
        this->id     = o.id;
        this->trader = o.trader;
        this->qty    = o.qty;
        this->price  = o.price;
        this->side   = o.side;
        this->ts_ns  = ts_ns;
    }
};

} // namespace MarketMicroStructure
