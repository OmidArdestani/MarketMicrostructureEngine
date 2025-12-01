#pragma once

#include "types.h"
#include <map>
#include <vector>
#include <deque>
#include <list>
#include <optional>
#include <unordered_map>

namespace MarketMicroStructure {

class OrderBook {
public:
    explicit OrderBook(SymbolId symbol);

    const SymbolId& symbol() const noexcept { return symbol_; }

    // Add a new resting order (no matching logic here)
    void addOrder(const BookOrder& ord);

    // Reduce or remove an existing order by id - O(1) lookup via index
    bool cancelOrder(OrderId id);

    // Match an incoming order against the book
    // Returns list of trades and leftover quantity (if any).
    std::pair<std::vector<Trade>*, Quantity> matchIncoming(const BookOrder& incoming, std::uint64_t ts_ns);

    // Query best bid / ask
    std::optional<BookLevel> bestBid() const;
    std::optional<BookLevel> bestAsk() const;

    // Depth snapshot (up to depth levels)
    std::vector<BookLevel> bids(std::size_t depth) const;
    std::vector<BookLevel> asks(std::size_t depth) const;

private:
    SymbolId symbol_;

    // Price -> queue of orders (time-priority per price)
    // For bids: highest price first; for asks: lowest price first.
    // Using std::list for O(1) erase with iterator
    using Queue = std::list<BookOrder>;
    using PriceLevel = std::map<Price, Queue, std::greater<Price>>;
    using PriceLevelAsk = std::map<Price, Queue, std::less<Price>>;

    PriceLevel bids_; // best bid at begin()
    PriceLevelAsk asks_; // best ask at begin()

    // Index for O(1) order lookup: OrderId -> (Side, Price, iterator into Queue)
    struct OrderLocation {
        Side side;
        Price price;
        Queue::iterator it;
    };
    std::unordered_map<OrderId, OrderLocation> order_index_;
};

} // namespace MarketMicroStructure
