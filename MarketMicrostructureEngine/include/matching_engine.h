#pragma once


#include "types.h"
#include "order_book.h"
#include "market_data_publisher.h"
#include <unordered_map>

namespace MarketMicroStructure {

class MatchingEngine {
public:
    explicit MatchingEngine(MarketDataPublisher& md_pub);

    void addSymbol(SymbolId symbol);

    // Handle a new order (market or limit)
    void handleNewOrder(const NewOrder& o, std::uint64_t ts_ns);

    // Handle cancel request - O(1) lookup via order index
    void handleCancel(const CancelOrder& c);

private:
    MarketDataPublisher& md_pub_;
    std::unordered_map<SymbolId, OrderBook> books_;
    
    // Index: OrderId -> SymbolId for O(1) cancel lookup
    std::unordered_map<OrderId, SymbolId> order_symbol_index_;
};

} // namespace MarketMicroStructure
