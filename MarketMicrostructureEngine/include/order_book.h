#pragma once

#include "types.h"
#include <map>
#include <vector>
#include <deque>
#include <optional>

namespace MarketMicroStructure {

class OrderBook {
public:
    explicit OrderBook(SymbolId symbol);

    const SymbolId& symbol() const noexcept { return symbol_; }

    // Add a new resting order (no matching logic here)
    void addOrder(const BookOrder& ord);

    // Reduce or remove an existing order by id
    bool cancelOrder(OrderId id);

    // Match an incoming order against the book
    // Returns list of trades and leftover quantity (if any).
    std::pair<std::vector<Trade>, Quantity> matchIncoming(const BookOrder& incoming, std::uint64_t ts_ns);

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
    using Queue = std::deque<BookOrder>;

    std::map<Price, Queue, std::greater<Price>> bids_; // best bid at begin()
    std::map<Price, Queue, std::less<Price>>    asks_; // best ask at begin()

    // Helper: locate order by id (linear scan for demo; could be indexed)
    bool removeFromSide(OrderId id, std::map<Price, Queue, std::greater<Price>>& side);
    bool removeFromSide(OrderId id, std::map<Price, Queue, std::less<Price>>&    side);
};

} // namespace MarketMicroStructure
