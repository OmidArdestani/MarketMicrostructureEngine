#include <matching_engine.h>
#include <ScopeTimer.hpp>

using namespace MarketMicroStructure;

MatchingEngine::MatchingEngine(MarketDataPublisher& md_pub)
    : md_pub_(md_pub)
{}

void MatchingEngine::addSymbol(SymbolId symbol) 
{
    books_.emplace(symbol, OrderBook{ symbol });
}

void MatchingEngine::handleNewOrder(const NewOrder& o, std::uint64_t ts_ns) 
{
    // O(1) lookup for the order book
    auto book_it = books_.find(o.symbol);
    if (book_it == books_.end())
        return;

    OrderBook& book = book_it->second;

    BookOrder incoming{
        o.id,
        o.trader,
        o.qty,
        o.price,
        o.side,
        ts_ns
    };

    // Market order: treat price as crossing "infinite"
    if (o.type == OrderType::Market) 
    {
        static Price MaxPrice{ std::numeric_limits<Price>::max() };
        static Price MinPrice{ std::numeric_limits<Price>::min() };

        if (o.side == Side::Buy)
        {
            // big price to cross all asks
            incoming.price = MaxPrice;
        } 
        else 
        {
            incoming.price = MinPrice;
        }
    }

    // Match against book
    auto [trades, remaining] = book.matchIncoming(incoming, ts_ns);

    // Publish trades and remove filled orders from the symbol index
    for (auto const& t : *trades)
    {
        md_pub_.publishTrade(t);
        // Remove fully filled resting orders from symbol index
        order_symbol_index_.erase(t.resting_id);
    }

    // If limit order has remaining qty & is allowed to rest
    if (o.type == OrderType::Limit && remaining > 0)
    {
        incoming.qty = remaining;
        book.addOrder(incoming);
        // Index the order for O(1) cancel lookup
        order_symbol_index_[o.id] = o.symbol;
    }

    // Publish top-of-book after each change
    TopOfBook tob;
    tob.symbol = o.symbol;
    if (auto best_bid = book.bestBid())
    {
        tob.best_bid = *best_bid;
        tob.valid = true;
    }
    if (auto best_ask = book.bestAsk())
    {
        tob.best_ask = *best_ask;
        tob.valid = tob.valid && true;
    }
    if (tob.valid) 
    {
        md_pub_.publishTopOfBook(tob);
    }
}

void MatchingEngine::handleCancel(const CancelOrder& c) 
{
    NScopeTimers::start("MatchingEngine::handleCancel");

    // O(1) lookup to find the symbol for this order
    auto idx_it = order_symbol_index_.find(c.id);
    if (idx_it == order_symbol_index_.end())
    {
        NScopeTimers::endAndLog("MatchingEngine::handleCancel");
        return; // Order not found
    }

    const SymbolId& symbol = idx_it->second;
    auto book_it = books_.find(symbol);
    if (book_it == books_.end())
    {
        NScopeTimers::endAndLog("MatchingEngine::handleCancel");
        return;
    }

    OrderBook& book = book_it->second;
    if (book.cancelOrder(c.id)) 
    {
        // Remove from symbol index
        order_symbol_index_.erase(idx_it);
        
        // After cancel we could publish new top-of-book
        TopOfBook tob;
        tob.symbol = symbol;
        if (auto best_bid = book.bestBid()) 
        {
            tob.best_bid = *best_bid;
            tob.valid = true;
        }
        if (auto best_ask = book.bestAsk()) 
        {
            tob.best_ask = *best_ask;
            tob.valid = tob.valid && true;
        }
        if (tob.valid) 
        {
            md_pub_.publishTopOfBook(tob);
        }
    }
    NScopeTimers::endAndLog("MatchingEngine::handleCancel");
}
